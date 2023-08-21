// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/stb/text_render_context_stb.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/allocation.h"
#include "impeller/core/allocator.h"
#include "impeller/typographer/font_glyph_pair.h"
#include "typeface_stb.h"

// These values can be customize per build.
// Glyph atlases are always square.
#ifndef MAX_GLYPH_ATLAS_SIZE
#define MAX_GLYPH_ATLAS_SIZE 2048u
#endif
#ifndef MIN_GLYPH_ATLAS_SIZE
#define MIN_GLYPH_ATLAS_SIZE 8u
#endif

#define DISABLE_COLOR_FONT_SUPPORT 1
#ifdef DISABLE_COLOR_FONT_SUPPORT
#define COLOR_FONT_BPP 1
#else
#define COLOR_FONT_BPP 4
#endif

// "Typical" conversion from font Points to Pixels.
// This assumes a constant pixels per em.
constexpr float POINTS_TO_PIXELS = 96.0/72.0;
// An available font scaling to improve rendering in the atlas if desired.
constexpr float FONT_RENDER_SCALING = 1.0;

// An stb rect packer instead of Skia based rect packer
struct STBRectPacker {
  // Standard rect pack context which can be used over N rect pack calls
  std::unique_ptr<stbrp_context> context;
  // Workspace which also must remain valid over N rect pack calls.
  // In the ideal the `nodes` array should be >= the width of the target rect.
  std::unique_ptr<stbrp_node[]> nodes;

  ~STBRectPacker() = default;

  STBRectPacker() = delete;

  STBRectPacker(int width, int height) 
    :context(std::make_unique<stbrp_context>())
    ,nodes(std::make_unique<stbrp_node[]>(width))
  {
    stbrp_init_target(context.get(),width,height,nodes.get(),width);
  }

  int PackRects(std::vector<stbrp_rect>& rects) {
    return stbrp_pack_rects(context.get(), rects.data(), (int)rects.size());
  }
};

// Analogous to the rect packer stored on the atlas context. But this uses STB.
static auto s_rect_packer = std::make_unique<STBRectPacker>(MIN_GLYPH_ATLAS_SIZE, MIN_GLYPH_ATLAS_SIZE);

// An simple bitmap in lieu of a skia bitmap.
struct STBBitmap {
  size_t width;
  size_t height;
  size_t bytes_per_pixel;
  std::unique_ptr<uint8_t[]> pixels;

  ~STBBitmap() = default;

  STBBitmap() = delete;

  STBBitmap(size_t width, size_t height, size_t bytes_per_pixel)
    :width(width)
    ,height(height)
    ,bytes_per_pixel(bytes_per_pixel)
    ,pixels(std::make_unique<uint8_t[]>(width*height*bytes_per_pixel)) {}

  uint8_t* const getPixels() const { return pixels.get() ; }

  uint8_t* getAddr(size_t x, size_t y) const {
    if(x >= width) return nullptr;
    if(y >= height) return nullptr;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    auto p = pixels.get();
    return &p[(x + width * y) * bytes_per_pixel];
  }

  size_t rowBytes() const { return width * bytes_per_pixel; }

  size_t stride() const { return rowBytes(); }

  size_t getWidth() const { return width; }

  size_t getHeight() const { return height; }

  size_t getSize() const { return width * height * bytes_per_pixel ; }
};

// Analogous to the bitmaps (one for each type) stored in each Atlas context.
static auto alpha_bitmap = std::make_shared<STBBitmap>(MIN_GLYPH_ATLAS_SIZE, MIN_GLYPH_ATLAS_SIZE, 1);
static auto color_bitmap = std::make_shared<STBBitmap>(MIN_GLYPH_ATLAS_SIZE, MIN_GLYPH_ATLAS_SIZE, COLOR_FONT_BPP);
static auto signed_distance_bitmap = std::make_shared<STBBitmap>(MIN_GLYPH_ATLAS_SIZE, MIN_GLYPH_ATLAS_SIZE, 1);

static std::shared_ptr<STBBitmap> get_atlas_bitmap(impeller::GlyphAtlas::Type type) {
  switch(type) {
    case impeller::GlyphAtlas::Type::kSignedDistanceField:
    {
      return signed_distance_bitmap;
      break;
    }
    case impeller::GlyphAtlas::Type::kAlphaBitmap:
    {
      return alpha_bitmap;
      break;
    }
    case impeller::GlyphAtlas::Type::kColorBitmap:
    {
      return color_bitmap ;
      break;
    }
  }
}

static void update_atlas_bitmap(std::shared_ptr<STBBitmap>& bitmap, impeller::GlyphAtlas::Type type) {
  switch(type) {
    case impeller::GlyphAtlas::Type::kSignedDistanceField:
    {
      signed_distance_bitmap = bitmap;
      break;
    }
    case impeller::GlyphAtlas::Type::kAlphaBitmap:
    {
      alpha_bitmap = bitmap;
      break;
    }
    case impeller::GlyphAtlas::Type::kColorBitmap:
    {
      color_bitmap = bitmap;
      break;
    }
  }
}

namespace impeller {

using FontGlyphPairRefVector =
    std::vector<std::reference_wrapper<const FontGlyphPair>>;

std::unique_ptr<TextRenderContext> TextRenderContext::Create(
    std::shared_ptr<Context> context) {
  // There is only one backend today.
  return std::make_unique<TextRenderContextSTB>(std::move(context));
}

constexpr auto kPadding = 1;

TextRenderContextSTB::TextRenderContextSTB(std::shared_ptr<Context> context)
    : TextRenderContext(std::move(context)) {}

TextRenderContextSTB::~TextRenderContextSTB() = default;

static FontGlyphPair::Set CollectUniqueFontGlyphPairs(
    GlyphAtlas::Type type,
    const TextRenderContext::FrameIterator& frame_iterator) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  FontGlyphPair::Set set;
  while (const TextFrame* frame = frame_iterator()) {
    for (const TextRun& run : frame->GetRuns()) {
      const Font& font = run.GetFont();
      // TODO(dnfield): If we're doing SDF here, we should be using a consistent
      // point size.
      // https://github.com/flutter/flutter/issues/112016
      for (const TextRun::GlyphPosition& glyph_position :
           run.GetGlyphPositions()) {
        set.insert({font, glyph_position.glyph});
      }
    }
  }
  return set;
}

// Function returns the count of "remaining pairs" not packed into rect of given size.
static size_t PairsFitInAtlasOfSize(
    const FontGlyphPair::Set& pairs,
    const ISize& atlas_size,
    std::vector<Rect>& glyph_positions,
    std::unique_ptr<STBRectPacker>& rect_packer) {
  if (atlas_size.IsEmpty()) {
    return false;
  }

  glyph_positions.clear();
  glyph_positions.reserve(pairs.size());

  size_t i = 0;
  std::vector<stbrp_rect> rect_packer_glyph_rects;
  for (auto it = pairs.begin(); it != pairs.end(); ++i, ++it) {
    const auto& pair = *it;

    const impeller::Font& font = pair.font;
    const impeller::Glyph& glyph = pair.glyph;
    const impeller::Font::Metrics& metrics = font.GetMetrics();
    auto typeface = font.GetTypeface();
    // We downcast to the correct typeface type to access `stb` specific methods.
    // NOTE: We use `static_pointer_cast` rather than `dynamic_cast` to obviate the
    // need for RTTI.
    std::shared_ptr<TypefaceSTB> typeface_stb = std::reinterpret_pointer_cast<TypefaceSTB>(typeface);
    // Conversion factor to scale font size in Points to pixels.
    // Note this assumes typical DPI.
    float text_size_pixels = metrics.point_size * POINTS_TO_PIXELS * FONT_RENDER_SCALING;

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    // NOTE: We increase the size of the glyph by one pixel in all dimensions to allow us to cut out padding later.
    float scale_y = stbtt_ScaleForPixelHeight(typeface_stb->GetFontInfo(), text_size_pixels );
    float scale_x = scale_y;
    stbtt_GetGlyphBitmapBox(typeface_stb->GetFontInfo(), glyph.index, scale_x, scale_y, &x0, &y0, &x1, &y1);
    auto width = x1 - x0;
    auto height = y1 - y0;

    // DEBUG
    //printf("**> font point size: %f scale: %f desired pixel height: %f \n", pair.font.GetMetrics().point_size, pair.font.GetMetrics().scale, text_size_pixels);
    //printf("**> glyph index: %u width: %d height: %d\n", pair.glyph.index, width, height);

    rect_packer_glyph_rects.push_back(stbrp_rect {
      .id = 0, // TODO: We might need to pack some id to trace back to the font/glyph pair.
      .w = width,
      .h = height,
      .x = 0,  // rect pack will fill this out
      .y = 0,  // rect pack will fill this out
      .was_packed = 0, // rect pack will fill this out
    });
  }

  size_t number_packed = 0;
  rect_packer->PackRects(rect_packer_glyph_rects);
  for(auto& packed_rect: rect_packer_glyph_rects) {
    if(packed_rect.was_packed) {
      glyph_positions.emplace_back(
        Rect::MakeXYWH(
          packed_rect.x + kPadding,
          packed_rect.y + kPadding,
          packed_rect.w - 2 * kPadding,
          packed_rect.h - 2 * kPadding
        )
      );
      number_packed++;
    }
  }
  // return the number NOT PACKED.
  return pairs.size() - number_packed;
}

static bool CanAppendToExistingAtlas(
    const std::shared_ptr<GlyphAtlas>& atlas,
    const FontGlyphPairRefVector& extra_pairs,
    std::vector<Rect>& glyph_positions,
    ISize atlas_size,
    std::unique_ptr<STBRectPacker>& rect_packer) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!rect_packer || atlas_size.IsEmpty()) {
    return false;
  }

  // We assume that all existing glyphs will fit. After all, they fit before.
  // The glyph_positions only contains the values for the additional glyphs
  // from extra_pairs.
  FML_DCHECK(glyph_positions.size() == 0);
  glyph_positions.reserve(extra_pairs.size());
  std::vector<stbrp_rect> rect_packer_glyph_rects;
  for (size_t i = 0; i < extra_pairs.size(); i++) {
    const FontGlyphPair& pair = extra_pairs[i];

    const impeller::Font& font = pair.font;
    const impeller::Glyph& glyph = pair.glyph;
    const impeller::Font::Metrics& metrics = font.GetMetrics();
    auto typeface = font.GetTypeface();
    // We downcast to the correct typeface type to access `stb` specific methods
    std::shared_ptr<TypefaceSTB> typeface_stb = std::dynamic_pointer_cast<TypefaceSTB>(typeface);
    // Conversion factor to scale font size in Points to pixels.
    // Note this assumes typical DPI.
    float text_size_pixels = metrics.point_size * POINTS_TO_PIXELS * FONT_RENDER_SCALING;

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    // NOTE: We increase the size of the glyph by one pixel in all dimensions to allow us to cut out padding later.
    float scale_y = stbtt_ScaleForPixelHeight(typeface_stb->GetFontInfo(), text_size_pixels );
    float scale_x = scale_y;
    stbtt_GetGlyphBitmapBox(typeface_stb->GetFontInfo(), glyph.index, scale_x, scale_y, &x0, &y0, &x1, &y1);
    auto width = x1 - x0;
    auto height = y1 - y0;

    // DEBUG:
    // printf("--> font point size: %f scale: %f desired pixel height: %f \n", pair.font.GetMetrics().point_size, pair.font.GetMetrics().scale, text_size_pixels);
    // printf("--> glyph index: %u width: %d height: %d\n", glyph.index, width, height);


    rect_packer_glyph_rects.push_back(stbrp_rect {
      .id = 0,
      .w = width,
      .h = height,
      .x = 0,  // rect pack will fill this out
      .y = 0,  // rect pack will fill this out
      .was_packed = 0, // rect pack will fill this out
    });
  }

  rect_packer->PackRects(rect_packer_glyph_rects);
  for(const auto& packed_rect: rect_packer_glyph_rects) {
    if(packed_rect.was_packed) {
      glyph_positions.emplace_back(
        Rect::MakeXYWH(
          packed_rect.x + kPadding,
          packed_rect.y + kPadding,
          packed_rect.w - 2 * kPadding,
          packed_rect.h - 2 * kPadding
        )
      );
    } else {
      return false;
    }
  }

  return true;
}

namespace {
ISize OptimumAtlasSizeForFontGlyphPairs(
    const FontGlyphPair::Set& pairs,
    std::vector<Rect>& glyph_positions,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context) {
  static constexpr auto kMinAtlasSize = MIN_GLYPH_ATLAS_SIZE;
  static constexpr auto kMaxAtlasSize = MAX_GLYPH_ATLAS_SIZE;

  TRACE_EVENT0("impeller", __FUNCTION__);

  ISize current_size(kMinAtlasSize, kMinAtlasSize);
  size_t total_pairs = pairs.size() + 1;
  do {

    auto rect_packer = std::make_unique<STBRectPacker>(current_size.width, current_size.height);

    auto remaining_pairs = PairsFitInAtlasOfSize(pairs, current_size,
                                                 glyph_positions, rect_packer);
    if (remaining_pairs == 0) {
      //atlas_context->UpdateRectPacker(rect_packer);
      s_rect_packer.swap(rect_packer);

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

/// Compute signed-distance field for an 8-bpp grayscale image (values greater
/// than 127 are considered "on") For details of this algorithm, see "The 'dead
/// reckoning' signed distance transform" [Grevera 2004]
static void ConvertBitmapToSignedDistanceField(uint8_t* pixels,
                                               uint16_t width,
                                               uint16_t height) {
  if (!pixels || width == 0 || height == 0) {
    return;
  }

  using ShortPoint = TPoint<uint16_t>;

  // distance to nearest boundary point map
  std::vector<Scalar> distance_map(width * height);
  // nearest boundary point map
  std::vector<ShortPoint> boundary_point_map(width * height);

  // Some helpers for manipulating the above arrays
#define image(_x, _y) (pixels[(_y)*width + (_x)] > 0x7f)
#define distance(_x, _y) distance_map[(_y)*width + (_x)]
#define nearestpt(_x, _y) boundary_point_map[(_y)*width + (_x)]

  const Scalar maxDist = hypot(width, height);
  const Scalar distUnit = 1;
  const Scalar distDiag = sqrt(2);

  // Initialization phase: set all distances to "infinity"; zero out nearest
  // boundary point map
  for (uint16_t y = 0; y < height; ++y) {
    for (uint16_t x = 0; x < width; ++x) {
      distance(x, y) = maxDist;
      nearestpt(x, y) = ShortPoint{0, 0};
    }
  }

  // Immediate interior/exterior phase: mark all points along the boundary as
  // such
  for (uint16_t y = 1; y < height - 1; ++y) {
    for (uint16_t x = 1; x < width - 1; ++x) {
      bool inside = image(x, y);
      if (image(x - 1, y) != inside || image(x + 1, y) != inside ||
          image(x, y - 1) != inside || image(x, y + 1) != inside) {
        distance(x, y) = 0;
        nearestpt(x, y) = ShortPoint{x, y};
      }
    }
  }

  // Forward dead-reckoning pass
  for (uint16_t y = 1; y < height - 2; ++y) {
    for (uint16_t x = 1; x < width - 2; ++x) {
      if (distance_map[(y - 1) * width + (x - 1)] + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x - 1, y - 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x, y - 1) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x, y - 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x + 1, y - 1) + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x + 1, y - 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x - 1, y) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x - 1, y);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
    }
  }

  // Backward dead-reckoning pass
  for (uint16_t y = height - 2; y >= 1; --y) {
    for (uint16_t x = width - 2; x >= 1; --x) {
      if (distance(x + 1, y) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x + 1, y);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x - 1, y + 1) + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x - 1, y + 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x, y + 1) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x, y + 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x + 1, y + 1) + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x + 1, y + 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
    }
  }

  // Interior distance negation pass; distances outside the figure are
  // considered negative
  // Also does final quantization.
  for (uint16_t y = 0; y < height; ++y) {
    for (uint16_t x = 0; x < width; ++x) {
      if (!image(x, y)) {
        distance(x, y) = -distance(x, y);
      }

      float norm_factor = 13.5;
      float dist = distance(x, y);
      float clamped_dist = fmax(-norm_factor, fmin(dist, norm_factor));
      float scaled_dist = clamped_dist / norm_factor;
      uint8_t quantized_value = ((scaled_dist + 1) / 2) * UINT8_MAX;
      pixels[y * width + x] = quantized_value;
    }
  }

#undef image
#undef distance
#undef nearestpt
}

//static void DrawGlyph(SkCanvas* canvas,
static void DrawGlyph(STBBitmap* bitmap,
                      const FontGlyphPair& font_glyph,
                      const Rect& location,
                      bool has_color) {
  const auto& metrics = font_glyph.font.GetMetrics();

  const impeller::Font& font = font_glyph.font;
  const impeller::Glyph& glyph = font_glyph.glyph;
  auto typeface = font.GetTypeface();
  // We downcast to the correct typeface type to access `stb` specific methods
  std::shared_ptr<TypefaceSTB> typeface_stb = std::dynamic_pointer_cast<TypefaceSTB>(typeface);
  // Conversion factor to scale font size in Points to pixels.
  // Note this assumes typical DPI.
  float text_size_pixels = metrics.point_size * POINTS_TO_PIXELS * FONT_RENDER_SCALING;
  float scale_y = stbtt_ScaleForPixelHeight(typeface_stb->GetFontInfo(), text_size_pixels);
  float scale_x = scale_y;

  auto output =  bitmap->getAddr(location.origin.x - kPadding, location.origin.y - kPadding);
  // For Alpha and Signed Distance field bitmaps we can use STB to draw the Glyph in place
  if(!has_color || DISABLE_COLOR_FONT_SUPPORT ) {
    stbtt_MakeGlyphBitmap(typeface_stb->GetFontInfo(), output, location.size.width +  2 * kPadding, location.size.height + 2 * kPadding, bitmap->stride(), scale_x, scale_y, glyph.index);
  } else {
    // But for color bitmaps we need to get the glyph pixels and then carry all channels into the atlas bitmap.
    // This may not be performant but I'm unsure of any other approach currently.
    int glyph_bitmap_width = 0;
    int glyph_bitmap_height = 0;
    int glyph_bitmap_xoff = 0;
    int glyph_bitmap_yoff = 0;
    auto glyph_pixels = stbtt_GetGlyphBitmap(typeface_stb->GetFontInfo(), scale_x, scale_y, glyph.index, &glyph_bitmap_width, &glyph_bitmap_height, &glyph_bitmap_xoff, &glyph_bitmap_yoff);

    uint8_t* write_pos = output;
    for(auto y = 0; y < glyph_bitmap_height; ++y) {
      for(auto x = 0; x < glyph_bitmap_width; ++x) {
        // Color bitmaps write as White (i.e. what is 0 in an alpha bitmap is 255 in a color bitmap)
        // But not alpha. Alpha still carries transparency info in the normal way.
        // TODO: There's some issue with color fonts, in that if the pixel color is nonzero, the alpha is ignored
        // during rendering. That is, partially (or fully) transparent pixels with nonzero color are rendered as fully opaque.
        uint8_t a = glyph_pixels[x + y * glyph_bitmap_width];
        uint8_t c = 255 - a;

        // Red channel
        *write_pos = c;
        write_pos++;
        // Green channel
        *write_pos = c;
        write_pos++;
        // Blue channel
        *write_pos = c;
        write_pos++;
        // Alpha channel
        *write_pos = a;
        write_pos++;
      }
      // next row
      write_pos = output + ( y * bitmap->stride());
    }
    stbtt_FreeBitmap(glyph_pixels, nullptr);
  }
}

static bool UpdateAtlasBitmap(const GlyphAtlas& atlas,
                              const std::shared_ptr<STBBitmap>& bitmap,
                              const FontGlyphPairRefVector& new_pairs) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  FML_DCHECK(bitmap != nullptr);

  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  for (const FontGlyphPair& pair : new_pairs) {
    auto pos = atlas.FindFontGlyphBounds(pair);
    if (!pos.has_value()) {
      continue;
    }
    DrawGlyph(bitmap.get(), pair, pos.value(), has_color);
  }
  return true;
}

static std::shared_ptr<STBBitmap> CreateAtlasBitmap(const GlyphAtlas& atlas,
                                                   const ISize& atlas_size) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  size_t bytes_per_pixel = 1;
  if(atlas.GetType() == GlyphAtlas::Type::kColorBitmap && !DISABLE_COLOR_FONT_SUPPORT) {
    bytes_per_pixel = COLOR_FONT_BPP;
  }
  auto bitmap = std::make_shared<STBBitmap>(atlas_size.width, atlas_size.height, bytes_per_pixel);

  bool has_color = atlas.GetType() == GlyphAtlas::Type::kColorBitmap;

  atlas.IterateGlyphs([&bitmap, has_color](const FontGlyphPair& font_glyph,
                                          const Rect& location) -> bool {
    DrawGlyph(bitmap.get(), font_glyph, location, has_color);
    return true;
  });

  return bitmap;
}

//static bool UpdateGlyphTextureAtlas(std::shared_ptr<SkBitmap> bitmap,
static bool UpdateGlyphTextureAtlas(std::shared_ptr<STBBitmap>& bitmap,
                                    const std::shared_ptr<Texture>& texture) {
  TRACE_EVENT0("impeller", __FUNCTION__);

  FML_DCHECK(bitmap != nullptr);

  auto texture_descriptor = texture->GetTextureDescriptor();

  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),  // data
      texture_descriptor.GetByteSizeOfBaseMipLevel()           // size
      // As the bitmap is static in this module I believe we don't need to specify a release proc.
  );

  return texture->SetContents(mapping);
}

static std::shared_ptr<Texture> UploadGlyphTextureAtlas(
    const std::shared_ptr<Allocator>& allocator,
    std::shared_ptr<STBBitmap>& bitmap,
    const ISize& atlas_size,
    PixelFormat format) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!allocator) {
    return nullptr;
  }

  FML_DCHECK(bitmap != nullptr);

  TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = StorageMode::kHostVisible;
  texture_descriptor.format = format;
  texture_descriptor.size = atlas_size;

  if (bitmap->rowBytes() * bitmap->getHeight() !=
      texture_descriptor.GetByteSizeOfBaseMipLevel()) {
    return nullptr;
  }

  auto texture = allocator->CreateTexture(texture_descriptor);
  if (!texture || !texture->IsValid()) {
    return nullptr;
  }
  texture->SetLabel("GlyphAtlas");

  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),  // data
      texture_descriptor.GetByteSizeOfBaseMipLevel()           // size
      // As the bitmap is static in this module I believe we don't need to specify a release proc.
  );

  if (!texture->SetContents(mapping)) {
    return nullptr;
  }
  return texture;
}

std::shared_ptr<GlyphAtlas> TextRenderContextSTB::CreateGlyphAtlas(
    GlyphAtlas::Type type,
    std::shared_ptr<GlyphAtlasContext> atlas_context,
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
                               s_rect_packer)) {
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
    //auto bitmap = atlas_context->GetBitmap();
    auto bitmap = get_atlas_bitmap(type);
    if (!UpdateAtlasBitmap(*last_atlas, bitmap, new_glyphs)) {
      return nullptr;
    }

    // ---------------------------------------------------------------------------
    // Step 6: Update the existing texture with the updated bitmap.
    // ---------------------------------------------------------------------------
    if (!UpdateGlyphTextureAtlas(bitmap, last_atlas->GetTexture())) {
      return nullptr;
    }
    return last_atlas;
  }
  // A new glyph atlas must be created.

  // ---------------------------------------------------------------------------
  // Step 4: Get the optimum size of the texture atlas.
  // ---------------------------------------------------------------------------
  auto glyph_atlas = std::make_shared<GlyphAtlas>(type);
  auto atlas_size = OptimumAtlasSizeForFontGlyphPairs(
      font_glyph_pairs, glyph_positions, atlas_context);

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
  auto bitmap = CreateAtlasBitmap(*glyph_atlas, atlas_size);
  if (!bitmap) {
    return nullptr;
  }

  update_atlas_bitmap(bitmap, type);

  // ---------------------------------------------------------------------------
  // Step 8: Upload the atlas as a texture.
  // ---------------------------------------------------------------------------
  PixelFormat format;
  switch (type) {
    case GlyphAtlas::Type::kSignedDistanceField:
      ConvertBitmapToSignedDistanceField(
          reinterpret_cast<uint8_t*>(bitmap->getPixels()), atlas_size.width,
          atlas_size.height);
    case GlyphAtlas::Type::kAlphaBitmap:
      format = PixelFormat::kA8UNormInt;
      break;
    case GlyphAtlas::Type::kColorBitmap:
      format = DISABLE_COLOR_FONT_SUPPORT ? PixelFormat::kA8UNormInt : PixelFormat::kR8G8B8A8UNormInt;
      break;
  }
  auto texture = UploadGlyphTextureAtlas(GetContext()->GetResourceAllocator(),
                                         bitmap, atlas_size, format);
  if (!texture) {
    return nullptr;
  }

  // ---------------------------------------------------------------------------
  // Step 9: Record the texture in the glyph atlas.
  // ---------------------------------------------------------------------------
  glyph_atlas->SetTexture(std::move(texture));

  return glyph_atlas;
}

}  // namespace impeller
