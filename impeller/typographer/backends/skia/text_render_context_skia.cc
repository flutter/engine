// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/text_render_context_skia.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/allocation.h"
#include "impeller/core/allocator.h"
#include "impeller/core/device_buffer.h"
#include "impeller/typographer/backends/skia/typeface_skia.h"
#include "impeller/typographer/rectangle_packer.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkFontMetrics.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace impeller {

using FontGlyphPairRefVector =
    std::vector<std::reference_wrapper<const FontGlyphPair>>;

std::unique_ptr<TextRenderContext> TextRenderContext::Create(
    std::shared_ptr<Context> context) {
  // There is only one backend today.
  return std::make_unique<TextRenderContextSkia>(std::move(context));
}

// TODO(bdero): We might be able to remove this per-glyph padding if we fix
//              the underlying causes of the overlap.
//              https://github.com/flutter/flutter/issues/114563
constexpr auto kPadding = 2;

std::optional<uint16_t> ComputeMinimumAlignment(
    const std::shared_ptr<Allocator>& allocator,
    const std::shared_ptr<const Capabilities>& capabilities,
    PixelFormat format) {
  if (!capabilities->SupportsSharedDeviceBufferTextureMemory()) {
    return std::nullopt;
  }
  return allocator->MinimumBytesPerRow(format);
}

TextRenderContextSkia::TextRenderContextSkia(std::shared_ptr<Context> context)
    : TextRenderContext(std::move(context)) {}

TextRenderContextSkia::~TextRenderContextSkia() = default;

static FontGlyphPair::Set CollectUniqueFontGlyphPairs(
    GlyphAtlas::Type type,
    const TextRenderContext::FrameIterator& frame_iterator) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  FontGlyphPair::Set set;
  while (const TextFrame* frame = frame_iterator()) {
    for (const TextRun& run : frame->GetRuns()) {
      const Font& font = run.GetFont();
      for (const TextRun::GlyphPosition& glyph_position :
           run.GetGlyphPositions()) {
        set.insert({font, glyph_position.glyph});
      }
    }
  }
  return set;
}

static size_t PairsFitInAtlasOfSize(
    const FontGlyphPair::Set& pairs,
    const ISize& atlas_size,
    std::vector<Rect>& glyph_positions,
    const std::shared_ptr<RectanglePacker>& rect_packer) {
  if (atlas_size.IsEmpty()) {
    return false;
  }

  glyph_positions.clear();
  glyph_positions.reserve(pairs.size());

  size_t i = 0;
  for (auto it = pairs.begin(); it != pairs.end(); ++i, ++it) {
    const auto& pair = *it;

    const auto glyph_size =
        ISize::Ceil((pair.glyph.bounds * pair.font.GetMetrics().scale).size);
    IPoint16 location_in_atlas;
    if (!rect_packer->addRect(glyph_size.width + kPadding,   //
                              glyph_size.height + kPadding,  //
                              &location_in_atlas             //
                              )) {
      return pairs.size() - i;
    }
    glyph_positions.emplace_back(Rect::MakeXYWH(location_in_atlas.x(),  //
                                                location_in_atlas.y(),  //
                                                glyph_size.width,       //
                                                glyph_size.height       //
                                                ));
  }

  return 0;
}

static bool CanAppendToExistingAtlas(
    const std::shared_ptr<GlyphAtlas>& atlas,
    const FontGlyphPairRefVector& extra_pairs,
    std::vector<Rect>& glyph_positions,
    ISize atlas_size,
    const std::shared_ptr<RectanglePacker>& rect_packer) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!rect_packer || atlas_size.IsEmpty()) {
    return false;
  }

  // We assume that all existing glyphs will fit. After all, they fit before.
  // The glyph_positions only contains the values for the additional glyphs
  // from extra_pairs.
  FML_DCHECK(glyph_positions.size() == 0);
  glyph_positions.reserve(extra_pairs.size());
  for (size_t i = 0; i < extra_pairs.size(); i++) {
    const FontGlyphPair& pair = extra_pairs[i];

    const auto glyph_size =
        ISize::Ceil((pair.glyph.bounds * pair.font.GetMetrics().scale).size);
    IPoint16 location_in_atlas;
    if (!rect_packer->addRect(glyph_size.width + kPadding,   //
                              glyph_size.height + kPadding,  //
                              &location_in_atlas             //
                              )) {
      return false;
    }
    glyph_positions.emplace_back(Rect::MakeXYWH(location_in_atlas.x(),  //
                                                location_in_atlas.y(),  //
                                                glyph_size.width,       //
                                                glyph_size.height       //
                                                ));
  }

  return true;
}

namespace {
ISize OptimumAtlasSizeForFontGlyphPairs(
    const FontGlyphPair::Set& pairs,
    std::vector<Rect>& glyph_positions,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    GlyphAtlas::Type type,
    std::optional<uint16_t> minimum_alignment) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  // This size needs to be above the minimum required aligment for linear
  // textures. This is 256 for older intel macs and decreases on iOS devices.
  static constexpr auto kMinAtlasSize = 256u;
  static constexpr auto kMinAlphaBitmapSize = 1024u;
  static constexpr auto kMaxAtlasSize = 4096u;
  uint16_t minimum_size = minimum_alignment.value_or(kMinAtlasSize);

  ISize current_size = type == GlyphAtlas::Type::kAlphaBitmap
                           ? ISize(kMinAlphaBitmapSize, kMinAlphaBitmapSize)
                           : ISize(minimum_size, minimum_size);
  size_t total_pairs = pairs.size() + 1;
  do {
    auto rect_packer = std::shared_ptr<RectanglePacker>(
        RectanglePacker::Factory(current_size.width, current_size.height));

    auto remaining_pairs = PairsFitInAtlasOfSize(pairs, current_size,
                                                 glyph_positions, rect_packer);
    if (remaining_pairs == 0) {
      atlas_context->UpdateRectPacker(rect_packer);
      return current_size;
    } else if (remaining_pairs < std::ceil(total_pairs / 2)) {
      current_size = ISize::MakeWH(
          std::max(current_size.width, current_size.height),
          Allocation::NextPowerOfTwoSize(
              std::min(current_size.width, current_size.height) + 1));
    } else {
      current_size = ISize::MakeWH(
          Allocation::NextPowerOfTwoSize(current_size.width + 1),
          Allocation::NextPowerOfTwoSize(current_size.height + 1));
    }
  } while (current_size.width <= kMaxAtlasSize &&
           current_size.height <= kMaxAtlasSize);
  return ISize{0, 0};
}
}  // namespace

static void DrawGlyph(SkCanvas* canvas,
                      const FontGlyphPair& font_glyph,
                      const Rect& location,
                      bool has_color) {
  const auto& metrics = font_glyph.font.GetMetrics();
  const auto position = SkPoint::Make(location.origin.x / metrics.scale,
                                      location.origin.y / metrics.scale);
  SkGlyphID glyph_id = font_glyph.glyph.index;

  SkFont sk_font(
      TypefaceSkia::Cast(*font_glyph.font.GetTypeface()).GetSkiaTypeface(),
      metrics.point_size, metrics.scaleX, metrics.skewX);
  sk_font.setEdging(SkFont::Edging::kAntiAlias);
  sk_font.setHinting(SkFontHinting::kSlight);
  sk_font.setEmbolden(metrics.embolden);

  auto glyph_color = has_color ? SK_ColorWHITE : SK_ColorBLACK;

  SkPaint glyph_paint;
  glyph_paint.setColor(glyph_color);
  canvas->resetMatrix();
  canvas->scale(metrics.scale, metrics.scale);
  canvas->drawGlyphs(
      1u,         // count
      &glyph_id,  // glyphs
      &position,  // positions
      SkPoint::Make(-font_glyph.glyph.bounds.GetLeft(),
                    -font_glyph.glyph.bounds.GetTop()),  // origin
      sk_font,                                           // font
      glyph_paint                                        // paint
  );
}

static bool UpdateAtlasBitmap(const GlyphAtlas& atlas,
                              const std::shared_ptr<SkBitmap>& bitmap,
                              const FontGlyphPairRefVector& new_pairs) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  FML_DCHECK(bitmap != nullptr);

  auto surface = SkSurfaces::WrapPixels(bitmap->pixmap());
  if (!surface) {
    return false;
  }
  auto canvas = surface->getCanvas();
  if (!canvas) {
    return false;
  }

  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  for (const FontGlyphPair& pair : new_pairs) {
    auto pos = atlas.FindFontGlyphBounds(pair);
    if (!pos.has_value()) {
      continue;
    }
    DrawGlyph(canvas, pair, pos.value(), has_color);
  }
  return true;
}

static std::pair<std::shared_ptr<SkBitmap>, std::shared_ptr<DeviceBuffer>>
CreateAtlasBitmap(const GlyphAtlas& atlas,
                  std::shared_ptr<Allocator> allocator,
                  const ISize& atlas_size) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  auto font_allocator = FontImpellerAllocator(std::move(allocator));
  auto bitmap = std::make_shared<SkBitmap>();
  SkImageInfo image_info;

  switch (atlas.GetType()) {
    case GlyphAtlas::Type::kAlphaBitmap:
      image_info = SkImageInfo::MakeA8(atlas_size.width, atlas_size.height);
      break;
    case GlyphAtlas::Type::kColorBitmap:
      image_info =
          SkImageInfo::MakeN32Premul(atlas_size.width, atlas_size.height);
      break;
  }

  bitmap->setInfo(image_info);
  if (!bitmap->tryAllocPixels(&font_allocator)) {
    return std::make_pair(nullptr, nullptr);
  }

  auto surface = SkSurfaces::WrapPixels(bitmap->pixmap());
  if (!surface) {
    return std::make_pair(nullptr, nullptr);
  }
  auto canvas = surface->getCanvas();
  if (!canvas) {
    return std::make_pair(nullptr, nullptr);
  }

  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  atlas.IterateGlyphs([canvas, has_color](const FontGlyphPair& font_glyph,
                                          const Rect& location) -> bool {
    DrawGlyph(canvas, font_glyph, location, has_color);
    return true;
  });

  auto device_buffer = font_allocator.GetDeviceBuffer();
  if (!device_buffer.has_value()) {
    return std::make_pair(nullptr, nullptr);
  }
  return std::make_pair(bitmap, device_buffer.value());
}

static bool UpdateGlyphTextureAtlas(std::shared_ptr<SkBitmap> bitmap,
                                    const std::shared_ptr<Texture>& texture) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  FML_DCHECK(bitmap != nullptr);
  auto texture_descriptor = texture->GetTextureDescriptor();

  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),  // data
      texture_descriptor.GetByteSizeOfBaseMipLevel(),           // size
      [bitmap](auto, auto) mutable { bitmap.reset(); }          // proc
  );

  return texture->SetContents(mapping);
}

static std::shared_ptr<Texture> UploadGlyphTextureAtlas(
    Allocator& allocator,
    const std::shared_ptr<DeviceBuffer>& device_buffer,
    const std::shared_ptr<SkBitmap>& bitmap,
    const ISize& atlas_size,
    PixelFormat format) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  FML_DCHECK(bitmap != nullptr);
  const auto& pixmap = bitmap->pixmap();

  TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = StorageMode::kHostVisible;
  texture_descriptor.format = format;
  texture_descriptor.size = atlas_size;

  // If the alignment isn't a multiple of the pixel format, we cannot use
  // a linear texture and instead must blit to a new texture.
  if (pixmap.rowBytes() * pixmap.height() !=
      texture_descriptor.GetByteSizeOfBaseMipLevel()) {
    return nullptr;
  }

  FML_DCHECK(allocator.MinimumBytesPerRow(format) <= pixmap.rowBytes());
  auto texture = device_buffer->AsTexture(allocator, texture_descriptor,
                                          texture_descriptor.GetBytesPerRow());
  if (!texture || !texture->IsValid()) {
    return nullptr;
  }
  texture->SetLabel("GlyphAtlas");
  return texture;
}

std::shared_ptr<GlyphAtlas> TextRenderContextSkia::CreateGlyphAtlas(
    GlyphAtlas::Type type,
    std::shared_ptr<GlyphAtlasContext> atlas_context,
    const std::shared_ptr<const Capabilities>& capabilities,
    FrameIterator frame_iterator) const {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!IsValid()) {
    return nullptr;
  }
  std::shared_ptr<GlyphAtlas> last_atlas = atlas_context->GetGlyphAtlas();

  // ---------------------------------------------------------------------------
  // Step 1: Collect unique font-glyph pairs in the frame.
  // ---------------------------------------------------------------------------

  FontGlyphPair::Set font_glyph_pairs =
      CollectUniqueFontGlyphPairs(type, frame_iterator);
  if (font_glyph_pairs.empty()) {
    return last_atlas;
  }

  // ---------------------------------------------------------------------------
  // Step 2: Determine if the atlas type and font glyph pairs are compatible
  //         with the current atlas and reuse if possible.
  // ---------------------------------------------------------------------------
  FontGlyphPairRefVector new_glyphs;
  for (const FontGlyphPair& pair : font_glyph_pairs) {
    if (!last_atlas->FindFontGlyphBounds(pair).has_value()) {
      new_glyphs.push_back(pair);
    }
  }
  if (last_atlas->GetType() == type && new_glyphs.size() == 0) {
    return last_atlas;
  }

  // ---------------------------------------------------------------------------
  // Step 3: Determine if the additional missing glyphs can be appended to the
  //         existing bitmap without recreating the atlas. This requires that
  //         the type is identical.
  // ---------------------------------------------------------------------------
  std::vector<Rect> glyph_positions;
  if (last_atlas->GetType() == type &&
      CanAppendToExistingAtlas(last_atlas, new_glyphs, glyph_positions,
                               atlas_context->GetAtlasSize(),
                               atlas_context->GetRectPacker())) {
    // The old bitmap will be reused and only the additional glyphs will be
    // added.

    // ---------------------------------------------------------------------------
    // Step 4: Record the positions in the glyph atlas of the newly added
    // glyphs.
    // ---------------------------------------------------------------------------
    for (size_t i = 0, count = glyph_positions.size(); i < count; i++) {
      last_atlas->AddTypefaceGlyphPosition(new_glyphs[i], glyph_positions[i]);
    }

    // ---------------------------------------------------------------------------
    // Step 5: Draw new font-glyph pairs into the existing bitmap.
    // ---------------------------------------------------------------------------
    auto [bitmap, device_buffer] = atlas_context->GetBitmap();
    if (!UpdateAtlasBitmap(*last_atlas, bitmap, new_glyphs)) {
      return nullptr;
    }

    // ---------------------------------------------------------------------------
    // Step 6: Update the existing texture with the updated bitmap.
    //         This is only necessary on backends that don't support creating
    //         a texture that shares memory with the underlying device buffer.
    // ---------------------------------------------------------------------------
    if (!capabilities->SupportsSharedDeviceBufferTextureMemory() &&
        !UpdateGlyphTextureAtlas(bitmap, last_atlas->GetTexture())) {
      return nullptr;
    }
    return last_atlas;
  }
  // A new glyph atlas must be created.
  PixelFormat format;
  switch (type) {
    case GlyphAtlas::Type::kAlphaBitmap:
      format = PixelFormat::kA8UNormInt;
      break;
    case GlyphAtlas::Type::kColorBitmap:
      format = PixelFormat::kR8G8B8A8UNormInt;
      break;
  }

  // ---------------------------------------------------------------------------
  // Step 4: Get the optimum size of the texture atlas.
  // ---------------------------------------------------------------------------
  auto glyph_atlas = std::make_shared<GlyphAtlas>(type);
  auto min_alignment = ComputeMinimumAlignment(
      GetContext()->GetResourceAllocator(), capabilities, format);
  auto atlas_size = OptimumAtlasSizeForFontGlyphPairs(
      font_glyph_pairs, glyph_positions, atlas_context, type, min_alignment);

  atlas_context->UpdateGlyphAtlas(glyph_atlas, atlas_size);
  if (atlas_size.IsEmpty()) {
    return nullptr;
  }
  // ---------------------------------------------------------------------------
  // Step 5: Find location of font-glyph pairs in the atlas. We have this from
  // the last step. So no need to do create another rect packer. But just do a
  // sanity check of counts. This could also be just an assertion as only a
  // construction issue would cause such a failure.
  // ---------------------------------------------------------------------------
  if (glyph_positions.size() != font_glyph_pairs.size()) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 6: Record the positions in the glyph atlas.
  // ---------------------------------------------------------------------------
  {
    size_t i = 0;
    for (auto it = font_glyph_pairs.begin(); it != font_glyph_pairs.end();
         ++i, ++it) {
      glyph_atlas->AddTypefaceGlyphPosition(*it, glyph_positions[i]);
    }
  }

  // ---------------------------------------------------------------------------
  // Step 7: Draw font-glyph pairs in the correct spot in the atlas.
  // ---------------------------------------------------------------------------
  auto [bitmap, device_buffer] = CreateAtlasBitmap(
      *glyph_atlas, GetContext()->GetResourceAllocator(), atlas_size);
  if (!bitmap) {
    return nullptr;
  }
  atlas_context->UpdateBitmap(bitmap, device_buffer);

  // ---------------------------------------------------------------------------
  // Step 8: Upload the atlas as a texture.
  // ---------------------------------------------------------------------------
  auto texture =
      UploadGlyphTextureAtlas(*GetContext()->GetResourceAllocator().get(),
                              device_buffer, bitmap, atlas_size, format);
  if (!texture) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 9: Record the texture in the glyph atlas.
  // ---------------------------------------------------------------------------
  glyph_atlas->SetTexture(std::move(texture));

  return glyph_atlas;
}

FontImpellerAllocator::FontImpellerAllocator(
    std::shared_ptr<impeller::Allocator> allocator)
    : allocator_(std::move(allocator)) {}

std::optional<std::shared_ptr<impeller::DeviceBuffer>>
FontImpellerAllocator::GetDeviceBuffer() const {
  return buffer_;
}

bool FontImpellerAllocator::allocPixelRef(SkBitmap* bitmap) {
  const SkImageInfo& info = bitmap->info();
  if (kUnknown_SkColorType == info.colorType() || info.width() < 0 ||
      info.height() < 0 || !info.validRowBytes(bitmap->rowBytes())) {
    return false;
  }

  DeviceBufferDescriptor descriptor;
  descriptor.storage_mode = StorageMode::kHostVisible;
  descriptor.size = ((bitmap->height() - 1) * bitmap->rowBytes()) +
                    (bitmap->width() * bitmap->bytesPerPixel());

  auto device_buffer = allocator_->CreateBuffer(descriptor);

  struct ImpellerPixelRef final : public SkPixelRef {
    ImpellerPixelRef(int w, int h, void* s, size_t r)
        : SkPixelRef(w, h, s, r) {}

    ~ImpellerPixelRef() override {}
  };

  auto pixel_ref = sk_sp<SkPixelRef>(
      new ImpellerPixelRef(info.width(), info.height(),
                           device_buffer->OnGetContents(), bitmap->rowBytes()));

  bitmap->setPixelRef(std::move(pixel_ref), 0, 0);
  buffer_ = std::move(device_buffer);
  return true;
}

}  // namespace impeller
