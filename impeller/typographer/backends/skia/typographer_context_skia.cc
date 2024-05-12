// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/typographer_context_skia.h"

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "fml/closure.h"

#include "impeller/base/allocation.h"
#include "impeller/core/allocator.h"
#include "impeller/core/buffer_view.h"
#include "impeller/core/formats.h"
#include "impeller/core/host_buffer.h"
#include "impeller/core/platform.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/geometry/size.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"
#include "impeller/typographer/backends/skia/typeface_skia.h"
#include "impeller/typographer/font_glyph_pair.h"
#include "impeller/typographer/glyph_atlas.h"
#include "impeller/typographer/rectangle_packer.h"
#include "impeller/typographer/typographer_context.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkSize.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace impeller {

// TODO(bdero): We might be able to remove this per-glyph padding if we fix
//              the underlying causes of the overlap.
//              https://github.com/flutter/flutter/issues/114563
constexpr auto kPadding = 2;

namespace {

class HostBufferAllocator : public SkBitmap::Allocator {
 public:
  explicit HostBufferAllocator(HostBuffer& host_buffer)
      : host_buffer_(host_buffer) {}

  [[nodiscard]] BufferView TakeBufferView() {
    buffer_view_.buffer->Flush();
    return std::move(buffer_view_);
  }

  // |SkBitmap::Allocator|
  bool allocPixelRef(SkBitmap* bitmap) override {
    if (!bitmap) {
      return false;
    }
    const SkImageInfo& info = bitmap->info();
    if (kUnknown_SkColorType == info.colorType() || info.width() < 0 ||
        info.height() < 0 || !info.validRowBytes(bitmap->rowBytes())) {
      return false;
    }

    size_t required_bytes = bitmap->rowBytes() * bitmap->height();
    BufferView buffer_view = host_buffer_.Emplace(nullptr, required_bytes,
                                                  DefaultUniformAlignment());

    // The impeller host buffer is not cleared between frames and may contain
    // stale data. The Skia software canvas does not write to pixels without
    // any contents, which causes this data to leak through.
    ::memset(buffer_view.buffer->OnGetContents() + buffer_view.range.offset, 0,
             required_bytes);

    auto pixel_ref = sk_sp<SkPixelRef>(new SkPixelRef(
        info.width(), info.height(),
        buffer_view.buffer->OnGetContents() + buffer_view.range.offset,
        bitmap->rowBytes()));

    bitmap->setPixelRef(std::move(pixel_ref), 0, 0);
    buffer_view_ = std::move(buffer_view);
    return true;
  }

 private:
  BufferView buffer_view_;
  HostBuffer& host_buffer_;
};

}  // namespace

std::shared_ptr<TypographerContext> TypographerContextSkia::Make() {
  return std::make_shared<TypographerContextSkia>();
}

TypographerContextSkia::TypographerContextSkia() = default;

TypographerContextSkia::~TypographerContextSkia() = default;

std::shared_ptr<GlyphAtlasContext>
TypographerContextSkia::CreateGlyphAtlasContext(GlyphAtlas::Type type) const {
  return std::make_shared<GlyphAtlasContext>(type);
}

static SkImageInfo GetImageInfo(const GlyphAtlas& atlas, Size size) {
  switch (atlas.GetType()) {
    case GlyphAtlas::Type::kAlphaBitmap:
      return SkImageInfo::MakeA8(SkISize{static_cast<int32_t>(size.width),
                                         static_cast<int32_t>(size.height)});
    case GlyphAtlas::Type::kColorBitmap:
      return SkImageInfo::MakeN32Premul(size.width, size.height);
  }
  FML_UNREACHABLE();
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
    if (!rect_packer->AddRect(glyph_size.width + kPadding,   //
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

/// Append as many glyphs to the texture as will fit, and return any
/// glyphs that did not.
static std::vector<FontGlyphPair> AppendToExistingAtlas(
    const std::shared_ptr<GlyphAtlas>& atlas,
    const std::vector<FontGlyphPair>& extra_pairs,
    std::vector<Rect>& glyph_positions,
    ISize atlas_size,
    const std::shared_ptr<RectanglePacker>& rect_packer) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!rect_packer || atlas_size.IsEmpty()) {
    return {};
  }

  // We assume that all existing glyphs will fit. After all, they fit before.
  // The glyph_positions only contains the values for the additional glyphs
  // from extra_pairs.
  FML_DCHECK(glyph_positions.size() == 0);
  // glyph_positions.reserve(extra_pairs.size());
  std::vector<FontGlyphPair> missing;
  bool stop_adding = false;

  for (size_t i = 0; i < extra_pairs.size(); i++) {
    const FontGlyphPair& pair = extra_pairs[i];

    if (stop_adding) {
      missing.push_back(pair);
      continue;
    }

    const auto glyph_size =
        ISize::Ceil(pair.glyph.bounds.GetSize() * pair.scaled_font.scale);
    IPoint16 location_in_atlas;
    if (!rect_packer->AddRect(glyph_size.width + kPadding,   //
                              glyph_size.height + kPadding,  //
                              &location_in_atlas             //
                              )) {
      missing.push_back(pair);
      stop_adding = true;
      continue;
    }
    glyph_positions.push_back(Rect::MakeXYWH(location_in_atlas.x(),  //
                                             location_in_atlas.y(),  //
                                             glyph_size.width,       //
                                             glyph_size.height       //
                                             ));
  }

  return missing;
}

static ISize OptimumAtlasSizeForFontGlyphPairs(
    const std::vector<FontGlyphPair>& pairs,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    const ISize& max_texture_size) {
  // Because we can't grow the skyline packer horizontally, pick a reasonable
  // large width for all atlases.
  static constexpr auto kAtlasWidth = 4096u;
  static constexpr auto kMinAtlasHeight = 1024u;

  TRACE_EVENT0("impeller", __FUNCTION__);

  std::vector<Rect> glyph_positions;
  ISize current_size = ISize(kAtlasWidth, kMinAtlasHeight);
  do {
    auto rect_packer = std::shared_ptr<RectanglePacker>(
        RectanglePacker::Factory(current_size.width, current_size.height));

    auto remaining_pairs = PairsFitInAtlasOfSize(pairs, current_size,
                                                 glyph_positions, rect_packer);
    if (remaining_pairs == 0) {
      atlas_context->UpdateRectPacker(rect_packer);
      return current_size;
    } else {
      current_size = ISize::MakeWH(
          kAtlasWidth, Allocation::NextPowerOfTwoSize(current_size.height + 1));
    }
  } while (current_size.width <= max_texture_size.width &&
           current_size.height <= max_texture_size.height);
  return ISize{0, 0};
}

static void DrawGlyph(SkCanvas* canvas,
                      const ScaledFont& scaled_font,
                      const Glyph& glyph,
                      bool has_color) {
  const auto& metrics = scaled_font.font.GetMetrics();
  const auto position = SkPoint::Make(0, 0);
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

  canvas->drawGlyphs(
      1u,         // count
      &glyph_id,  // glyphs
      &position,  // positions
      SkPoint::Make(-glyph.bounds.GetLeft(), -glyph.bounds.GetTop()),  // origin
      sk_font,                                                         // font
      glyph_paint                                                      // paint
  );
}

static bool UpdateAtlasBitmap(const GlyphAtlas& atlas,
                              std::shared_ptr<BlitPass>& blit_pass,
                              HostBuffer& host_buffer,
                              const std::shared_ptr<Texture>& texture,
                              const std::vector<FontGlyphPair>& new_pairs) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  for (const FontGlyphPair& pair : new_pairs) {
    auto pos = atlas.FindFontGlyphBounds(pair);
    if (!pos.has_value()) {
      continue;
    }
    Size size = pos->GetSize();
    if (size.IsEmpty()) {
      continue;
    }

    SkBitmap bitmap;
    HostBufferAllocator allocator(host_buffer);
    bitmap.setInfo(GetImageInfo(atlas, size));
    if (!bitmap.tryAllocPixels(&allocator)) {
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

    DrawGlyph(canvas, pair.scaled_font, pair.glyph, has_color);

    if (!blit_pass->AddCopy(allocator.TakeBufferView(), texture,
                            IRect::MakeXYWH(pos->GetLeft(), pos->GetTop(),
                                            size.width, size.height))) {
      return false;
    }
  }
  return true;
}

std::shared_ptr<GlyphAtlas> TypographerContextSkia::CreateGlyphAtlas(
    Context& context,
    GlyphAtlas::Type type,
    HostBuffer& host_buffer,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    const FontGlyphMap& font_glyph_map) const {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!IsValid()) {
    return nullptr;
  }
  std::shared_ptr<GlyphAtlas> last_atlas = atlas_context->GetGlyphAtlas();
  FML_DCHECK(last_atlas->GetType() == type);

  if (font_glyph_map.empty()) {
    return last_atlas;
  }

  std::shared_ptr<CommandBuffer> cmd_buffer = context.CreateCommandBuffer();
  std::shared_ptr<BlitPass> blit_pass = cmd_buffer->CreateBlitPass();

  fml::ScopedCleanupClosure closure([&]() {
    blit_pass->EncodeCommands(context.GetResourceAllocator());
    context.GetCommandQueue()->Submit({std::move(cmd_buffer)});
  });

  // ---------------------------------------------------------------------------
  // Step 1: Determine if the atlas type and font glyph pairs are compatible
  //         with the current atlas and reuse if possible.
  // ---------------------------------------------------------------------------
  std::vector<FontGlyphPair> new_glyphs;
  for (const auto& font_value : font_glyph_map) {
    const ScaledFont& scaled_font = font_value.first;
    const FontGlyphAtlas* font_glyph_atlas =
        last_atlas->GetFontGlyphAtlas(scaled_font.font, scaled_font.scale);
    if (font_glyph_atlas) {
      for (const Glyph& glyph : font_value.second) {
        if (!font_glyph_atlas->FindGlyphBounds(glyph)) {
          new_glyphs.emplace_back(scaled_font, glyph);
        }
      }
    } else {
      for (const Glyph& glyph : font_value.second) {
        new_glyphs.emplace_back(scaled_font, glyph);
      }
    }
  }
  if (new_glyphs.size() == 0) {
    return last_atlas;
  }

  // ---------------------------------------------------------------------------
  // Step 2: Determine if the additional missing glyphs can be appended to the
  //         existing bitmap without recreating the atlas.
  // ---------------------------------------------------------------------------
  std::vector<Rect> glyph_positions;
  std::vector<FontGlyphPair> missing_pairs;
  size_t i = 0;
  if (last_atlas->GetTexture()) {
    // Append all glyphs that fit into the current atlas.
    missing_pairs = AppendToExistingAtlas(
        last_atlas, new_glyphs, glyph_positions, atlas_context->GetAtlasSize(),
        atlas_context->GetRectPacker());

    // ---------------------------------------------------------------------------
    // Step 3a: Record the positions in the glyph atlas of the newly added
    //          glyphs.
    // ---------------------------------------------------------------------------
    for (size_t count = glyph_positions.size(); i < count; i++) {
      last_atlas->AddTypefaceGlyphPosition(new_glyphs[i], glyph_positions[i]);
    }

    // ---------------------------------------------------------------------------
    // Step 4a: Draw new font-glyph pairs into the a host buffer and encode
    // the uploads into the blit pass.
    // ---------------------------------------------------------------------------
    if (!UpdateAtlasBitmap(*last_atlas, blit_pass, host_buffer,
                           last_atlas->GetTexture(), new_glyphs)) {
      return nullptr;
    }

    // If all glyphs fit, just return the old atlas.
    if (missing_pairs.empty()) {
      return last_atlas;
    }
  }

  // A new glyph atlas must be created. Compute the size needed by all of the
  // glyphs that couldn't be added to the existing atlas.
  ISize atlas_size = OptimumAtlasSizeForFontGlyphPairs(
      last_atlas->GetTexture()
          ? missing_pairs
          : new_glyphs,  //                                         //
      atlas_context,     //
      context.GetResourceAllocator()->GetMaxTextureSizeSupported()  //
  );

  atlas_context->UpdateGlyphAtlas(last_atlas, atlas_size);
  if (atlas_size.IsEmpty()) {
    return nullptr;
  }

  // Compute the new atlas size.
  ISize new_atlas_size;
  if (last_atlas->GetTexture()) {
    new_atlas_size =
        ISize{atlas_size.width,
              atlas_size.height + last_atlas->GetTexture()->GetSize().height};
  } else {
    new_atlas_size = atlas_size;
  }

  if (atlas_context->GetRectPacker()) {
    auto new_rect_packer =
        atlas_context->GetRectPacker()->Clone(new_atlas_size.height);
    atlas_context->UpdateRectPacker(std::move(new_rect_packer));
  } else {
    atlas_context->UpdateRectPacker(
        RectanglePacker::Factory(new_atlas_size.width, new_atlas_size.height));
  }

  TextureDescriptor descriptor;
  switch (type) {
    case GlyphAtlas::Type::kAlphaBitmap:
      descriptor.format =
          context.GetCapabilities()->GetDefaultGlyphAtlasFormat();
      break;
    case GlyphAtlas::Type::kColorBitmap:
      descriptor.format = PixelFormat::kR8G8B8A8UNormInt;
      break;
  }
  descriptor.size = new_atlas_size;
  descriptor.storage_mode = StorageMode::kDevicePrivate;
  std::shared_ptr<Texture> new_texture =
      context.GetResourceAllocator()->CreateTexture(descriptor);
  new_texture->SetLabel("GlyphAtlas");

  // The texture needs to be cleared to transparent black so that linearly
  // samplex rotated/skewed glyphs do not grab uninitialized data. We could
  // instead use a render pass to clear to transparent black, but there are
  // more restrictions on what kinds of textures can be bound on GLES.
  {
    auto bytes =
        new_texture->GetTextureDescriptor().GetByteSizeOfBaseMipLevel();
    BufferView buffer_view =
        host_buffer.Emplace(nullptr, bytes, DefaultUniformAlignment());

    ::memset(buffer_view.buffer->OnGetContents() + buffer_view.range.offset, 0,
             bytes);
    blit_pass->AddCopy(buffer_view, new_texture);
  }

  // Blit the old texture to the top left of the new atlas.
  if (last_atlas->GetTexture()) {
    blit_pass->AddCopy(last_atlas->GetTexture(), new_texture,
                       IRect::MakeSize(last_atlas->GetTexture()->GetSize()),
                       {0, 0});
  }

  // Now append all remaining glyphs. This should never have any missing data...
  last_atlas->SetTexture(std::move(new_texture));
  auto expect_empty = AppendToExistingAtlas(
      last_atlas, last_atlas->GetTexture() ? missing_pairs : new_glyphs,
      glyph_positions, atlas_context->GetAtlasSize(),
      atlas_context->GetRectPacker());
  if (!expect_empty.empty()) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 3a: Record the positions in the glyph atlas of the newly added
  //          glyphs.
  // ---------------------------------------------------------------------------
  for (size_t count = glyph_positions.size(); i < count; i++) {
    last_atlas->AddTypefaceGlyphPosition(new_glyphs[i], glyph_positions[i]);
  }

  // ---------------------------------------------------------------------------
  // Step 4a: Draw new font-glyph pairs into the a host buffer and encode
  // the uploads into the blit pass.
  // ---------------------------------------------------------------------------
  if (!UpdateAtlasBitmap(*last_atlas, blit_pass, host_buffer,
                         last_atlas->GetTexture(), new_glyphs)) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 8b: Record the texture in the glyph atlas.
  // ---------------------------------------------------------------------------

  return last_atlas;
}

}  // namespace impeller
