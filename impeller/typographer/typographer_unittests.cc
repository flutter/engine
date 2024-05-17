// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display_list/dl_color.h"
#include "flutter/display_list/testing/dl_test_snippets.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "impeller/core/host_buffer.h"
#include "impeller/playground/playground.h"
#include "impeller/playground/playground_test.h"
#include "impeller/typographer/backends/skia/text_frame_skia.h"
#include "impeller/typographer/backends/skia/typographer_context_skia.h"
#include "impeller/typographer/font_glyph_pair.h"
#include "impeller/typographer/lazy_glyph_atlas.h"
#include "impeller/typographer/rectangle_packer.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkTextBlob.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "txt/platform.h"

// TODO(zanderso): https://github.com/flutter/flutter/issues/127701
// NOLINTBEGIN(bugprone-unchecked-optional-access)

namespace impeller {
namespace testing {

using TypographerTest = PlaygroundTest;
INSTANTIATE_PLAYGROUND_SUITE(TypographerTest);

static std::shared_ptr<GlyphAtlas> CreateGlyphAtlas(
    Context& context,
    const TypographerContext* typographer_context,
    HostBuffer& host_buffer,
    GlyphAtlas::Type type,
    Scalar scale,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    const TextFrame& frame) {
  FontGlyphMap font_glyph_map;
  frame.CollectUniqueFontGlyphPairs(font_glyph_map, scale);
  return typographer_context->CreateGlyphAtlas(context, type, host_buffer,
                                               atlas_context, font_glyph_map);
}

static std::shared_ptr<GlyphAtlas> CreateGlyphAtlas(
    Context& context,
    const TypographerContext* typographer_context,
    HostBuffer& host_buffer,
    GlyphAtlas::Type type,
    Scalar scale,
    const std::shared_ptr<GlyphAtlasContext>& atlas_context,
    const std::vector<std::shared_ptr<TextFrame>>& frames) {
  FontGlyphMap font_glyph_map;
  for (auto& frame : frames) {
    frame->CollectUniqueFontGlyphPairs(font_glyph_map, scale);
  }
  return typographer_context->CreateGlyphAtlas(context, type, host_buffer,
                                               atlas_context, font_glyph_map);
}

TEST_P(TypographerTest, CanConvertTextBlob) {
  SkFont font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString(
      "the quick brown fox jumped over the lazy dog.", font);
  ASSERT_TRUE(blob);
  auto frame = MakeTextFrameFromTextBlobSkia(blob);
  ASSERT_EQ(frame->GetRunCount(), 1u);
  for (const auto& run : frame->GetRuns()) {
    ASSERT_TRUE(run.IsValid());
    ASSERT_EQ(run.GetGlyphCount(), 45u);
  }
}

TEST_P(TypographerTest, CanCreateRenderContext) {
  auto context = TypographerContextSkia::Make();
  ASSERT_TRUE(context && context->IsValid());
}

TEST_P(TypographerTest, CanCreateGlyphAtlas) {
  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kAlphaBitmap);
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  ASSERT_TRUE(context && context->IsValid());
  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString("hello", sk_font);
  ASSERT_TRUE(blob);
  auto atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob));
  ASSERT_NE(atlas, nullptr);
  ASSERT_NE(atlas->GetTexture(), nullptr);
  ASSERT_EQ(atlas->GetType(), GlyphAtlas::Type::kAlphaBitmap);
  ASSERT_EQ(atlas->GetGlyphCount(), 4llu);

  std::optional<impeller::ScaledFont> first_scaled_font;
  std::optional<impeller::Glyph> first_glyph;
  Rect first_rect;
  atlas->IterateGlyphs([&](const ScaledFont& scaled_font, const Glyph& glyph,
                           const Rect& rect) -> bool {
    first_scaled_font = scaled_font;
    first_glyph = glyph;
    first_rect = rect;
    return false;
  });

  ASSERT_TRUE(first_scaled_font.has_value());
  ASSERT_TRUE(atlas
                  ->FindFontGlyphBounds(
                      {first_scaled_font.value(), first_glyph.value()})
                  .has_value());
}

TEST_P(TypographerTest, LazyAtlasTracksColor) {
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
#if FML_OS_MACOSX
  auto mapping = flutter::testing::OpenFixtureAsSkData("Apple Color Emoji.ttc");
#else
  auto mapping = flutter::testing::OpenFixtureAsSkData("NotoColorEmoji.ttf");
#endif
  ASSERT_TRUE(mapping);
  sk_sp<SkFontMgr> font_mgr = txt::GetDefaultFontManager();
  SkFont emoji_font(font_mgr->makeFromData(mapping), 50.0);
  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);

  auto blob = SkTextBlob::MakeFromString("hello", sk_font);
  ASSERT_TRUE(blob);
  auto frame = MakeTextFrameFromTextBlobSkia(blob);

  ASSERT_FALSE(frame->GetAtlasType() == GlyphAtlas::Type::kColorBitmap);

  LazyGlyphAtlas lazy_atlas(TypographerContextSkia::Make());

  lazy_atlas.AddTextFrame(*frame, 1.0f);

  frame = MakeTextFrameFromTextBlobSkia(
      SkTextBlob::MakeFromString("😀 ", emoji_font));

  ASSERT_TRUE(frame->GetAtlasType() == GlyphAtlas::Type::kColorBitmap);

  lazy_atlas.AddTextFrame(*frame, 1.0f);

  // Creates different atlases for color and red bitmap.
  auto color_atlas = lazy_atlas.CreateOrGetGlyphAtlas(
      *GetContext(), *host_buffer, GlyphAtlas::Type::kColorBitmap);

  auto bitmap_atlas = lazy_atlas.CreateOrGetGlyphAtlas(
      *GetContext(), *host_buffer, GlyphAtlas::Type::kAlphaBitmap);

  ASSERT_FALSE(color_atlas == bitmap_atlas);
}

TEST_P(TypographerTest, GlyphAtlasWithOddUniqueGlyphSize) {
  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kAlphaBitmap);
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  ASSERT_TRUE(context && context->IsValid());
  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString("AGH", sk_font);
  ASSERT_TRUE(blob);
  auto atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob));
  ASSERT_NE(atlas, nullptr);
  ASSERT_NE(atlas->GetTexture(), nullptr);

  EXPECT_EQ(atlas->GetTexture()->GetSize().width, 4096u);
  EXPECT_EQ(atlas->GetTexture()->GetSize().height, 1024u);
}

TEST_P(TypographerTest, GlyphAtlasIsRecycledIfUnchanged) {
  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kAlphaBitmap);
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  ASSERT_TRUE(context && context->IsValid());
  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString("spooky skellingtons", sk_font);
  ASSERT_TRUE(blob);
  auto atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob));
  ASSERT_NE(atlas, nullptr);
  ASSERT_NE(atlas->GetTexture(), nullptr);
  ASSERT_EQ(atlas, atlas_context->GetGlyphAtlas());

  // now attempt to re-create an atlas with the same text blob.

  auto next_atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob));
  ASSERT_EQ(atlas, next_atlas);
  ASSERT_EQ(atlas_context->GetGlyphAtlas(), atlas);
}

TEST_P(TypographerTest, GlyphAtlasWithLotsOfdUniqueGlyphSize) {
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kAlphaBitmap);
  ASSERT_TRUE(context && context->IsValid());

  const char* test_string =
      "QWERTYUIOPASDFGHJKLZXCVBNMqewrtyuiopasdfghjklzxcvbnm,.<>[]{};':"
      "2134567890-=!@#$%^&*()_+"
      "œ∑´®†¥¨ˆøπ““‘‘åß∂ƒ©˙∆˚¬…æ≈ç√∫˜µ≤≥≥≥≥÷¡™£¢∞§¶•ªº–≠⁄€‹›ﬁﬂ‡°·‚—±Œ„´‰Á¨Ø∏”’/"
      "* Í˝ */¸˛Ç◊ı˜Â¯˘¿";

  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString(test_string, sk_font);
  ASSERT_TRUE(blob);

  FontGlyphMap font_glyph_map;
  size_t size_count = 8;
  for (size_t index = 0; index < size_count; index += 1) {
    MakeTextFrameFromTextBlobSkia(blob)->CollectUniqueFontGlyphPairs(
        font_glyph_map, 0.6 * index);
  };
  auto atlas =
      context->CreateGlyphAtlas(*GetContext(), GlyphAtlas::Type::kAlphaBitmap,
                                *host_buffer, atlas_context, font_glyph_map);
  ASSERT_NE(atlas, nullptr);
  ASSERT_NE(atlas->GetTexture(), nullptr);

  std::set<uint16_t> unique_glyphs;
  std::vector<uint16_t> total_glyphs;
  atlas->IterateGlyphs(
      [&](const ScaledFont& scaled_font, const Glyph& glyph, const Rect& rect) {
        unique_glyphs.insert(glyph.index);
        total_glyphs.push_back(glyph.index);
        return true;
      });

  EXPECT_EQ(unique_glyphs.size() * size_count, atlas->GetGlyphCount());
  EXPECT_EQ(total_glyphs.size(), atlas->GetGlyphCount());

  EXPECT_TRUE(atlas->GetGlyphCount() > 0);
  EXPECT_TRUE(atlas->GetTexture()->GetSize().width > 0);
  EXPECT_TRUE(atlas->GetTexture()->GetSize().height > 0);
}

TEST_P(TypographerTest, GlyphAtlasTextureIsRecycledIfUnchanged) {
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kAlphaBitmap);
  ASSERT_TRUE(context && context->IsValid());
  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString("spooky 1", sk_font);
  ASSERT_TRUE(blob);
  auto atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob));
  auto old_packer = atlas_context->GetRectPacker();

  ASSERT_NE(atlas, nullptr);
  ASSERT_NE(atlas->GetTexture(), nullptr);
  ASSERT_EQ(atlas, atlas_context->GetGlyphAtlas());

  auto* first_texture = atlas->GetTexture().get();

  // Now create a new glyph atlas with a nearly identical blob.

  auto blob2 = SkTextBlob::MakeFromString("spooky 2", sk_font);
  auto next_atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob2));
  ASSERT_EQ(atlas, next_atlas);
  auto* second_texture = next_atlas->GetTexture().get();

  auto new_packer = atlas_context->GetRectPacker();

  ASSERT_EQ(second_texture, first_texture);
  ASSERT_EQ(old_packer, new_packer);
}

TEST_P(TypographerTest, MaybeHasOverlapping) {
  sk_sp<SkFontMgr> font_mgr = txt::GetDefaultFontManager();
  sk_sp<SkTypeface> typeface =
      font_mgr->matchFamilyStyle("Arial", SkFontStyle::Normal());
  SkFont sk_font(typeface, 0.5f);

  auto frame =
      MakeTextFrameFromTextBlobSkia(SkTextBlob::MakeFromString("1", sk_font));
  // Single character has no overlapping
  ASSERT_FALSE(frame->MaybeHasOverlapping());

  auto frame_2 = MakeTextFrameFromTextBlobSkia(
      SkTextBlob::MakeFromString("123456789", sk_font));
  ASSERT_FALSE(frame_2->MaybeHasOverlapping());
}

TEST_P(TypographerTest, GlyphColorIsPartOfCacheKey) {
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
#if FML_OS_MACOSX
  auto mapping = flutter::testing::OpenFixtureAsSkData("Apple Color Emoji.ttc");
#else
  auto mapping = flutter::testing::OpenFixtureAsSkData("NotoColorEmoji.ttf");
#endif
  ASSERT_TRUE(mapping);
  sk_sp<SkFontMgr> font_mgr = txt::GetDefaultFontManager();
  SkFont emoji_font(font_mgr->makeFromData(mapping), 50.0);

  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kColorBitmap);

  // Create two frames with the same character and a different color, expect
  // that it adds a character.
  auto frame = MakeTextFrameFromTextBlobSkia(
      SkTextBlob::MakeFromString("😂", emoji_font), flutter::DlColor::kCyan());
  auto frame_2 = MakeTextFrameFromTextBlobSkia(
      SkTextBlob::MakeFromString("😂", emoji_font),
      flutter::DlColor::kMagenta());

  auto next_atlas = CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                                     GlyphAtlas::Type::kColorBitmap, 1.0f,
                                     atlas_context, {frame, frame_2});

  EXPECT_EQ(next_atlas->GetGlyphCount(), 2u);
}

TEST_P(TypographerTest, GlyphColorIsIgnoredForNonEmojiFonts) {
  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  sk_sp<SkFontMgr> font_mgr = txt::GetDefaultFontManager();
  sk_sp<SkTypeface> typeface =
      font_mgr->matchFamilyStyle("Arial", SkFontStyle::Normal());
  SkFont sk_font(typeface, 0.5f);

  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kColorBitmap);

  // Create two frames with the same character and a different color, but as a
  // non-emoji font the text frame constructor will ignore it.
  auto frame = MakeTextFrameFromTextBlobSkia(
      SkTextBlob::MakeFromString("A", sk_font), flutter::DlColor::kCyan());
  auto frame_2 = MakeTextFrameFromTextBlobSkia(
      SkTextBlob::MakeFromString("A", sk_font), flutter::DlColor::kMagenta());

  auto next_atlas = CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                                     GlyphAtlas::Type::kColorBitmap, 1.0f,
                                     atlas_context, {frame, frame_2});

  EXPECT_EQ(next_atlas->GetGlyphCount(), 1u);
}

TEST_P(TypographerTest, RectanglePackerAddsNonoverlapingRectangles) {
  auto packer = RectanglePacker::Factory(200, 100);
  ASSERT_NE(packer, nullptr);
  ASSERT_EQ(packer->PercentFull(), 0);

  const SkIRect packer_area = SkIRect::MakeXYWH(0, 0, 200, 100);

  IPoint16 first_output = {-1, -1};  // Fill with sentinel values
  ASSERT_TRUE(packer->AddRect(20, 20, &first_output));
  // Make sure the rectangle is placed such that it is inside the bounds of
  // the packer's area.
  const SkIRect first_rect =
      SkIRect::MakeXYWH(first_output.x(), first_output.y(), 20, 20);
  ASSERT_TRUE(SkIRect::Intersects(packer_area, first_rect));

  // Initial area was 200 x 100 = 20_000
  // We added 20x20 = 400. 400 / 20_000 == 0.02 == 2%
  ASSERT_TRUE(flutter::testing::NumberNear(packer->PercentFull(), 0.02));

  IPoint16 second_output = {-1, -1};
  ASSERT_TRUE(packer->AddRect(140, 90, &second_output));
  const SkIRect second_rect =
      SkIRect::MakeXYWH(second_output.x(), second_output.y(), 140, 90);
  // Make sure the rectangle is placed such that it is inside the bounds of
  // the packer's area but not in the are of the first rectangle.
  ASSERT_TRUE(SkIRect::Intersects(packer_area, second_rect));
  ASSERT_FALSE(SkIRect::Intersects(first_rect, second_rect));

  // We added another 90 x 140 = 12_600 units, now taking us to 13_000
  // 13_000 / 20_000 == 0.65 == 65%
  ASSERT_TRUE(flutter::testing::NumberNear(packer->PercentFull(), 0.65));

  // There's enough area to add this rectangle, but no space big enough for
  // the 50 units of width.
  IPoint16 output;
  ASSERT_FALSE(packer->AddRect(50, 50, &output));
  // Should be unchanged.
  ASSERT_TRUE(flutter::testing::NumberNear(packer->PercentFull(), 0.65));

  packer->Reset();
  // Should be empty now.
  ASSERT_EQ(packer->PercentFull(), 0);
}

TEST(TypographerTest, RectanglePackerFillsRows) {
  auto skyline = RectanglePacker::Factory(257, 256);

  // Fill up the first row.
  IPoint16 loc;
  for (auto i = 0u; i < 16; i++) {
    skyline->AddRect(16, 16, &loc);
  }
  // Last rectangle still in first row.
  EXPECT_EQ(loc.x(), 256 - 16);
  EXPECT_EQ(loc.y(), 0);

  // Fill up second row.
  for (auto i = 0u; i < 16; i++) {
    skyline->AddRect(16, 16, &loc);
  }

  EXPECT_EQ(loc.x(), 256 - 16);
  EXPECT_EQ(loc.y(), 16);
}

TEST_P(TypographerTest, GlyphAtlasTextureWillGrowTilMaxTextureSize) {
  if (GetBackend() == PlaygroundBackend::kOpenGLES) {
    GTEST_SKIP() << "Atlas growth isn't supported for OpenGLES currently.";
  }

  auto host_buffer = HostBuffer::Create(GetContext()->GetResourceAllocator());
  auto context = TypographerContextSkia::Make();
  auto atlas_context =
      context->CreateGlyphAtlasContext(GlyphAtlas::Type::kAlphaBitmap);
  ASSERT_TRUE(context && context->IsValid());
  SkFont sk_font = flutter::testing::CreateTestFontOfSize(12);
  auto blob = SkTextBlob::MakeFromString("A", sk_font);
  ASSERT_TRUE(blob);
  auto atlas =
      CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                       GlyphAtlas::Type::kAlphaBitmap, 1.0f, atlas_context,
                       *MakeTextFrameFromTextBlobSkia(blob));
  // Continually append new glyphs until the glyph size grows to the maximum.
  // Note that the sizes here are more or less experimentally determined, but
  // the important expectation is that the atlas size will shrink again after
  // growing to the maximum size.
  constexpr ISize expected_sizes[13] = {
      {4096, 4096},   //
      {4096, 4096},   //
      {4096, 8192},   //
      {4096, 8192},   //
      {4096, 8192},   //
      {4096, 8192},   //
      {4096, 16384},  //
      {4096, 16384},  //
      {4096, 16384},  //
      {4096, 16384},  //
      {4096, 16384},  //
      {4096, 16384},  //
      {4096, 4096}    // Shrinks!
  };

  for (int i = 0; i < 13; i++) {
    SkFont sk_font = flutter::testing::CreateTestFontOfSize(50 + i);
    auto blob = SkTextBlob::MakeFromString("A", sk_font);

    atlas =
        CreateGlyphAtlas(*GetContext(), context.get(), *host_buffer,
                         GlyphAtlas::Type::kAlphaBitmap, 50 + i, atlas_context,
                         *MakeTextFrameFromTextBlobSkia(blob));
    ASSERT_TRUE(!!atlas);
    EXPECT_EQ(atlas->GetTexture()->GetTextureDescriptor().size,
              expected_sizes[i]);
  }
}

}  // namespace testing
}  // namespace impeller

// NOLINTEND(bugprone-unchecked-optional-access)
