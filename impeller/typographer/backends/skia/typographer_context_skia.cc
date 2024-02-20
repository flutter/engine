// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/typographer_context_skia.h"

#include <numeric>
#include <unordered_set>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/allocation.h"
#include "impeller/core/allocator.h"
#include "impeller/core/buffer_view.h"
#include "impeller/core/device_buffer.h"
#include "impeller/core/device_buffer_descriptor.h"
#include "impeller/core/formats.h"
#include "impeller/typographer/backends/skia/glyph_atlas_context_skia.h"
#include "impeller/typographer/backends/skia/typeface_skia.h"
#include "impeller/typographer/font_glyph_pair.h"
#include "impeller/typographer/rectangle_packer.h"
#include "impeller/typographer/typographer_context.h"
#include "include/core/SkColor.h"
#include "include/core/SkSize.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkMallocPixelRef.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkSamplingOptions.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace impeller {

// TODO(bdero): We might be able to remove this per-glyph padding if we fix
//              the underlying causes of the overlap.
//              https://github.com/flutter/flutter/issues/114563
constexpr auto kPadding = 2;

class ImpellerAllocator : public SkBitmap::Allocator {
 public:
  explicit ImpellerAllocator(std::shared_ptr<impeller::Allocator> allocator)
      : allocator_(std::move(allocator)) {}

  std::shared_ptr<impeller::DeviceBuffer> GetDeviceBuffer() const {
    return buffer_;
  }

  bool allocPixelRef(SkBitmap* bitmap) {
    const SkImageInfo& info = bitmap->info();
    if (kUnknown_SkColorType == info.colorType() || info.width() < 0 ||
        info.height() < 0 || !info.validRowBytes(bitmap->rowBytes())) {
      return false;
    }

    impeller::DeviceBufferDescriptor descriptor;
    descriptor.storage_mode = impeller::StorageMode::kHostVisible;
    descriptor.size = ((bitmap->height() - 1) * bitmap->rowBytes()) +
                      (bitmap->width() * bitmap->bytesPerPixel());

    std::shared_ptr<impeller::DeviceBuffer> device_buffer =
        allocator_->CreateBuffer(descriptor);

    struct ImpellerPixelRef final : public SkPixelRef {
      ImpellerPixelRef(int w, int h, void* s, size_t r)
          : SkPixelRef(w, h, s, r) {}

      ~ImpellerPixelRef() override {}
    };

    auto pixel_ref = sk_sp<SkPixelRef>(new ImpellerPixelRef(
        info.width(), info.height(), device_buffer->OnGetContents(),
        bitmap->rowBytes()));

    bitmap->setPixelRef(std::move(pixel_ref), 0, 0);
    buffer_ = std::move(device_buffer);
    return true;
  }

  std::shared_ptr<DeviceBuffer> buffer_;
  std::shared_ptr<impeller::Allocator> allocator_;
};

std::shared_ptr<TypographerContext> TypographerContextSkia::Make() {
  return std::make_shared<TypographerContextSkia>();
}

TypographerContextSkia::TypographerContextSkia() = default;

TypographerContextSkia::~TypographerContextSkia() = default;

std::shared_ptr<GlyphAtlasContext>
TypographerContextSkia::CreateGlyphAtlasContext() const {
  return std::make_shared<GlyphAtlasContextSkia>();
}

static size_t PairsFitInAtlasOfSize(
    const std::vector<FontGlyphPair>& pairs,
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
        ISize::Ceil(pair.glyph.bounds.GetSize() * pair.scaled_font.scale);
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
    const std::vector<FontGlyphPair>& extra_pairs,
    const std::vector<FontGlyphPair>& all_glyphs,
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
  std::vector<FontGlyphPair> missing_pairs;
  bool broken = false;
  for (size_t i = 0; i < extra_pairs.size(); i++) {
    const FontGlyphPair& pair = extra_pairs[i];

    const auto glyph_size =
        ISize::Ceil(pair.glyph.bounds.GetSize() * pair.scaled_font.scale);
    IPoint16 location_in_atlas;
    if (broken || !rect_packer->addRect(glyph_size.width + kPadding,   //
                                        glyph_size.height + kPadding,  //
                                        &location_in_atlas             //
                                        )) {
      missing_pairs.push_back(pair);
      broken = true;
      continue;
    }
    glyph_positions.emplace_back(Rect::MakeXYWH(location_in_atlas.x(),  //
                                                location_in_atlas.y(),  //
                                                glyph_size.width,       //
                                                glyph_size.height       //
                                                ));
  }

  // Some glyphs may not have fit. Next we'll check if any existing glyphs
  // can be evicted to make room for new glyphs. This generally works well
  // for emojis as they generally all have rectangular bounds, and if they're
  // the same font size they'll mostly be the same shape.
  if (!missing_pairs.empty()) {
    if (missing_pairs.size() > 36) {
      return false;
    }
    std::vector<std::pair<Rect, FontGlyphPair>> free_spots = {};
    atlas->IterateGlyphs([&all_glyphs, &free_spots](const ScaledFont& font,
                                                    const Glyph& glyph,
                                                    const Rect& bounds) {
      if (std::find_if(all_glyphs.begin(), all_glyphs.end(),
                       [&font, glyph](const FontGlyphPair& pair) {
                         return pair.scaled_font.font.IsEqual(font.font) &&
                                pair.scaled_font.scale == font.scale &&
                                glyph.index == pair.glyph.index;
                       }) == all_glyphs.end()) {
        free_spots.push_back(
            std::make_pair(bounds, FontGlyphPair{font, glyph}));
      }
      return true;
    });
    if (free_spots.empty()) {
      return false;
    }
    for (const auto& pair : missing_pairs) {
      const auto glyph_size =
          ISize::Ceil(pair.glyph.bounds.GetSize() * pair.scaled_font.scale);
      size_t j = 0u;
      bool found = false;
      for (; j < free_spots.size(); j++) {
        const auto& [bounds, free_pair] = free_spots[j];
        if (glyph_size.width <= bounds.GetWidth() &&
            glyph_size.height <= bounds.GetHeight()) {
          found = true;
          break;
        }
      }
      if (found) {
        glyph_positions.emplace_back(free_spots[j].first);
        atlas->RemoveTypefaceGlyphPosition(free_spots[j].second);
        free_spots[j].first = Rect::MakeLTRB(0, 0, 0, 0);
      } else {
        return false;
      }
    }
  }
  return true;
}

static ISize OptimumAtlasSizeForFontGlyphPairs(
    const std::vector<FontGlyphPair>& pairs,
    std::vector<Rect>& glyph_positions,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    GlyphAtlas::Type type,
    const ISize& max_texture_size) {
  static constexpr auto kMinAtlasSize = 8u;
  static constexpr auto kMinAlphaBitmapSize = 1024u;

  TRACE_EVENT0("impeller", __FUNCTION__);

  ISize current_size = type == GlyphAtlas::Type::kAlphaBitmap
                           ? ISize(kMinAlphaBitmapSize, kMinAlphaBitmapSize)
                           : ISize(kMinAtlasSize, kMinAtlasSize);
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
  } while (current_size.width <= max_texture_size.width &&
           current_size.height <= max_texture_size.height);
  return ISize{0, 0};
}

static void DrawGlyph(SkCanvas* canvas,
                      const ScaledFont& scaled_font,
                      const Glyph& glyph,
                      const Rect& location,
                      bool has_color) {
  const auto& metrics = scaled_font.font.GetMetrics();
  const auto position = SkPoint::Make(location.GetX() / scaled_font.scale,
                                      location.GetY() / scaled_font.scale);
  SkGlyphID glyph_id = glyph.index;

  SkFont sk_font(
      TypefaceSkia::Cast(*scaled_font.font.GetTypeface()).GetSkiaTypeface(),
      metrics.point_size, metrics.scaleX, metrics.skewX);
  sk_font.setEdging(SkFont::Edging::kAntiAlias);
  sk_font.setHinting(SkFontHinting::kSlight);
  sk_font.setEmbolden(metrics.embolden);

  auto glyph_color = has_color ? SK_ColorWHITE : SK_ColorBLACK;

  SkPaint glyph_paint;
  glyph_paint.setColor(glyph_color);
  canvas->resetMatrix();
  canvas->scale(scaled_font.scale, scaled_font.scale);
  canvas->drawGlyphs(1u,         // count
                     &glyph_id,  // glyphs
                     &position,  // positions
                     SkPoint::Make(-glyph.bounds.GetLeft(),
                                   -glyph.bounds.GetTop()),  // origin
                     sk_font,                                // font
                     glyph_paint                             // paint
  );
}

static bool UpdateAtlasBitmap(const GlyphAtlas& atlas,
                              const std::vector<FontGlyphPair>& new_pairs,
                              const std::shared_ptr<Texture>& texture,
                              const std::shared_ptr<Allocator>& allocator) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  std::vector<Texture::ContentUpdate> updates;
  for (const FontGlyphPair& pair : new_pairs) {
    auto pos = atlas.FindFontGlyphBounds(pair);
    if (!pos.has_value()) {
      continue;
    }
    auto rect = pos.value();
    if (rect.IsEmpty()) {
      continue;
    }
    auto bounds = ISize(static_cast<int32_t>(std::ceil(rect.GetWidth())),
                        static_cast<int32_t>(std::ceil(rect.GetHeight())));

    SkImageInfo image_info;
    switch (atlas.GetType()) {
      case GlyphAtlas::Type::kAlphaBitmap:
        image_info =
            SkImageInfo::MakeA8(SkISize::Make(bounds.width, bounds.height));
        break;
      case GlyphAtlas::Type::kColorBitmap:
        image_info = SkImageInfo::MakeN32Premul(bounds.width, bounds.height);
        break;
    }

    auto bitmap = SkBitmap();
    bitmap.setInfo(image_info);
    if (!bitmap.tryAllocPixels()) {
      return false;
    }

    auto surface = SkSurfaces::WrapPixels(bitmap.pixmap());
    if (!surface) {
      return false;
    }
    auto canvas = surface->getCanvas();
    if (!canvas) {
      return false;
    }
    DrawGlyph(canvas, pair.scaled_font, pair.glyph,
              Rect::MakeLTRB(0, 0, rect.GetWidth(), rect.GetHeight()),
              has_color);

    const auto& pixmap = bitmap.pixmap();
    DeviceBufferDescriptor desc;
    desc.storage_mode = StorageMode::kHostVisible;
    desc.size = pixmap.rowBytes() * pixmap.height();
    auto device_buffer = allocator->CreateBuffer(desc);
    if (!device_buffer->CopyHostBuffer(
            reinterpret_cast<uint8_t*>(bitmap.getAddr(0, 0)), {0, desc.size})) {
      return false;
    }

    updates.push_back(Texture::ContentUpdate{
        .region = IRect::MakeLTRB(rect.GetX(), rect.GetY(), rect.GetRight(),
                                  rect.GetBottom()),
        .buffer_view = DeviceBuffer::AsBufferView(device_buffer),
    });
  }

  if (!texture->SetContents(updates.data(), updates.size())) {
    return false;
  }

  return true;
}

static std::shared_ptr<SkBitmap> CreateAtlasBitmap(const GlyphAtlas& atlas,
                                                   const ISize& atlas_size) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  auto bitmap = std::make_shared<SkBitmap>();
  SkImageInfo image_info;

  switch (atlas.GetType()) {
    case GlyphAtlas::Type::kAlphaBitmap:
      image_info =
          SkImageInfo::MakeA8(SkISize{static_cast<int32_t>(atlas_size.width),
                                      static_cast<int32_t>(atlas_size.height)});
      break;
    case GlyphAtlas::Type::kColorBitmap:
      image_info =
          SkImageInfo::MakeN32Premul(atlas_size.width, atlas_size.height);
      break;
  }

  if (!bitmap->tryAllocPixels(image_info)) {
    return nullptr;
  }

  auto surface = SkSurfaces::WrapPixels(bitmap->pixmap());
  if (!surface) {
    return nullptr;
  }
  auto canvas = surface->getCanvas();
  if (!canvas) {
    return nullptr;
  }

  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  atlas.IterateGlyphs([canvas, has_color](const ScaledFont& scaled_font,
                                          const Glyph& glyph,
                                          const Rect& location) -> bool {
    DrawGlyph(canvas, scaled_font, glyph, location, has_color);
    return true;
  });

  return bitmap;
}

static bool UpdateGlyphTextureAtlas(std::shared_ptr<SkBitmap> bitmap,
                                    const std::shared_ptr<Texture>& texture) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  FML_DCHECK(bitmap != nullptr);
  auto texture_descriptor = texture->GetTextureDescriptor();

  return texture->SetContents(
      reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),
      texture_descriptor.GetByteSizeOfBaseMipLevel());
}

static std::shared_ptr<Texture> UploadGlyphTextureAtlas(
    const std::shared_ptr<Allocator>& allocator,
    std::shared_ptr<SkBitmap> bitmap,
    const ISize& atlas_size,
    PixelFormat format) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!allocator) {
    return nullptr;
  }

  FML_DCHECK(bitmap != nullptr);
  const auto& pixmap = bitmap->pixmap();

  TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = StorageMode::kDevicePrivate;
  texture_descriptor.format = format;
  texture_descriptor.size = atlas_size;

  if (pixmap.rowBytes() * pixmap.height() !=
      texture_descriptor.GetByteSizeOfBaseMipLevel()) {
    return nullptr;
  }

  auto texture = allocator->CreateTexture(texture_descriptor);
  if (!texture || !texture->IsValid()) {
    return nullptr;
  }
  texture->SetLabel("GlyphAtlas");

  if (!texture->SetContents(
          reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),
          texture_descriptor.GetByteSizeOfBaseMipLevel())) {
    return nullptr;
  }
  return texture;
}

std::shared_ptr<GlyphAtlas> TypographerContextSkia::CreateGlyphAtlas(
    Context& context,
    GlyphAtlas::Type type,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    const FontGlyphMap& font_glyph_map) const {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!IsValid()) {
    return nullptr;
  }
  auto& atlas_context_skia = GlyphAtlasContextSkia::Cast(*atlas_context);
  std::shared_ptr<GlyphAtlas> last_atlas = atlas_context->GetGlyphAtlas();

  if (font_glyph_map.empty()) {
    return last_atlas;
  }

  // ---------------------------------------------------------------------------
  // Step 1: Determine if the atlas type and font glyph pairs are compatible
  //         with the current atlas and reuse if possible.
  // ---------------------------------------------------------------------------
  std::vector<FontGlyphPair> new_glyphs;
  std::vector<FontGlyphPair> all_glyphs;
  for (const auto& font_value : font_glyph_map) {
    const ScaledFont& scaled_font = font_value.first;
    const FontGlyphAtlas* font_glyph_atlas =
        last_atlas->GetFontGlyphAtlas(scaled_font.font, scaled_font.scale);
    if (font_glyph_atlas) {
      for (const Glyph& glyph : font_value.second) {
        all_glyphs.emplace_back(FontGlyphPair{scaled_font, glyph});
        if (!font_glyph_atlas->FindGlyphBounds(glyph)) {
          new_glyphs.emplace_back(scaled_font, glyph);
        }
      }
    } else {
      for (const Glyph& glyph : font_value.second) {
        new_glyphs.emplace_back(scaled_font, glyph);
        all_glyphs.emplace_back(FontGlyphPair{scaled_font, glyph});
      }
    }
  }
  if (last_atlas->GetType() == type && new_glyphs.size() == 0) {
    return last_atlas;
  }

  // ---------------------------------------------------------------------------
  // Step 2: Determine if the additional missing glyphs can be appended to the
  //         existing bitmap without recreating the atlas. This requires that
  //         the type is identical.
  // ---------------------------------------------------------------------------
  std::vector<Rect> glyph_positions;
  if (last_atlas->GetType() == type &&
      CanAppendToExistingAtlas(last_atlas, new_glyphs, all_glyphs,
                               glyph_positions, atlas_context->GetAtlasSize(),
                               atlas_context->GetRectPacker())) {
    // The old bitmap will be reused and only the additional glyphs will be
    // added.

    // ---------------------------------------------------------------------------
    // Step 3a: Record the positions in the glyph atlas of the newly added
    //          glyphs.
    // ---------------------------------------------------------------------------
    for (size_t i = 0, count = glyph_positions.size(); i < count; i++) {
      last_atlas->AddTypefaceGlyphPosition(new_glyphs[i], glyph_positions[i]);
    }

    // ---------------------------------------------------------------------------
    // Step 4a: Draw new font-glyph pairs into the existing bitmap.
    // ---------------------------------------------------------------------------
    if (!UpdateAtlasBitmap(*last_atlas, new_glyphs, last_atlas->GetTexture(),
                           context.GetResourceAllocator())) {
      return nullptr;
    }
    return last_atlas;
  }
  // A new glyph atlas must be created.

  // ---------------------------------------------------------------------------
  // Step 3b: Get the optimum size of the texture atlas.
  // ---------------------------------------------------------------------------
  std::vector<FontGlyphPair> font_glyph_pairs;
  font_glyph_pairs.reserve(std::accumulate(
      font_glyph_map.begin(), font_glyph_map.end(), 0,
      [](const int a, const auto& b) { return a + b.second.size(); }));
  for (const auto& font_value : font_glyph_map) {
    const ScaledFont& scaled_font = font_value.first;
    for (const Glyph& glyph : font_value.second) {
      font_glyph_pairs.push_back({scaled_font, glyph});
    }
  }
  auto glyph_atlas = std::make_shared<GlyphAtlas>(type);
  auto atlas_size = OptimumAtlasSizeForFontGlyphPairs(
      font_glyph_pairs,                                             //
      glyph_positions,                                              //
      atlas_context,                                                //
      type,                                                         //
      context.GetResourceAllocator()->GetMaxTextureSizeSupported()  //
  );

  atlas_context->UpdateGlyphAtlas(glyph_atlas, atlas_size);
  if (atlas_size.IsEmpty()) {
    return nullptr;
  }
  // ---------------------------------------------------------------------------
  // Step 4b: Find location of font-glyph pairs in the atlas. We have this from
  // the last step. So no need to do create another rect packer. But just do a
  // sanity check of counts. This could also be just an assertion as only a
  // construction issue would cause such a failure.
  // ---------------------------------------------------------------------------
  if (glyph_positions.size() != font_glyph_pairs.size()) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 5b: Record the positions in the glyph atlas.
  // ---------------------------------------------------------------------------
  {
    size_t i = 0;
    for (auto it = font_glyph_pairs.begin(); it != font_glyph_pairs.end();
         ++i, ++it) {
      glyph_atlas->AddTypefaceGlyphPosition(*it, glyph_positions[i]);
    }
  }

  // ---------------------------------------------------------------------------
  // Step 6b: Draw font-glyph pairs in the correct spot in the atlas.
  // ---------------------------------------------------------------------------
  auto bitmap = CreateAtlasBitmap(*glyph_atlas, atlas_size);
  if (!bitmap) {
    return nullptr;
  }
  atlas_context_skia.UpdateBitmap(bitmap);

  // If the new atlas size is the same size as the previous texture, reuse the
  // texture and treat this as an updated that replaces all glyphs.
  if (last_atlas && last_atlas->GetTexture()) {
    std::shared_ptr<Texture> last_texture = last_atlas->GetTexture();
    if (atlas_size == last_texture->GetSize()) {
      if (!UpdateGlyphTextureAtlas(bitmap, last_texture)) {
        return nullptr;
      }

      glyph_atlas->SetTexture(last_texture);
      return glyph_atlas;
    }
  }

  // ---------------------------------------------------------------------------
  // Step 7b: Upload the atlas as a texture.
  // ---------------------------------------------------------------------------
  PixelFormat format;
  switch (type) {
    case GlyphAtlas::Type::kAlphaBitmap:
      format = context.GetCapabilities()->GetDefaultGlyphAtlasFormat();
      break;
    case GlyphAtlas::Type::kColorBitmap:
      format = PixelFormat::kR8G8B8A8UNormInt;
      break;
  }
  auto texture = UploadGlyphTextureAtlas(context.GetResourceAllocator(), bitmap,
                                         atlas_size, format);
  if (!texture) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 8b: Record the texture in the glyph atlas.
  // ---------------------------------------------------------------------------
  glyph_atlas->SetTexture(std::move(texture));

  return glyph_atlas;
}

}  // namespace impeller
