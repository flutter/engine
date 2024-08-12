// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display_list/display_list.h"
#include "display_list/dl_blend_mode.h"
#include "display_list/dl_builder.h"
#include "display_list/dl_color.h"
#include "display_list/dl_paint.h"
#include "display_list/dl_tile_mode.h"
#include "display_list/effects/dl_color_filter.h"
#include "display_list/effects/dl_image_filter.h"
#include "display_list/effects/dl_mask_filter.h"
#include "flutter/impeller/aiks/aiks_unittests.h"

#include "impeller/playground/widgets.h"
#include "include/core/SkRRect.h"
#include "third_party/imgui/imgui.h"

////////////////////////////////////////////////////////////////////////////////
// This is for tests of Canvas that are interested the results of rendering
// blurs.
////////////////////////////////////////////////////////////////////////////////

namespace impeller {
namespace testing {

using namespace flutter;

TEST_P(AiksTest, CanRenderMaskBlurHugeSigma) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kGreen());
  paint.setMaskFilter(DlBlurMaskFilter::Make(DlBlurStyle::kNormal, 99999));
  builder.DrawCircle({400, 400}, 300, paint);
  builder.Restore();

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderForegroundBlendWithMaskBlur) {
  // This case triggers the ForegroundPorterDuffBlend path. The color filter
  // should apply to the color only, and respect the alpha mask.
  DisplayListBuilder builder;
  builder.ClipRect(SkRect::MakeXYWH(100, 150, 400, 400));

  DlPaint paint;
  paint.setColor(DlColor::kWhite());
  paint.setMaskFilter(
      DlBlurMaskFilter::Make(DlBlurStyle::kNormal, Sigma(Radius(20)).sigma));
  paint.setColorFilter(
      DlBlendColorFilter::Make(DlColor::kGreen(), DlBlendMode::kSrc));
  builder.DrawCircle({400, 400}, 200, paint);
  builder.Restore();

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderForegroundAdvancedBlendWithMaskBlur) {
  // This case triggers the ForegroundAdvancedBlend path. The color filter
  // should apply to the color only, and respect the alpha mask.
  DisplayListBuilder builder;
  builder.ClipRect(SkRect::MakeXYWH(100, 150, 400, 400));

  DlPaint paint;
  paint.setColor(DlColor::kLightGrey());
  paint.setMaskFilter(
      DlBlurMaskFilter::Make(DlBlurStyle::kNormal, Sigma(Radius(20)).sigma));
  builder.DrawCircle({400, 400}, 200, paint);
  builder.Restore();

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderBackdropBlurInteractive) {
  auto callback = [&]() -> sk_sp<DisplayList> {
    static PlaygroundPoint point_a(Point(50, 50), 30, Color::White());
    static PlaygroundPoint point_b(Point(300, 200), 30, Color::White());
    auto [a, b] = DrawPlaygroundLine(point_a, point_b);

    DisplayListBuilder builder;
    DlPaint paint;
    paint.setColor(DlColor::kCornflowerBlue());
    builder.DrawCircle({100, 100}, 50, paint);

    paint.setColor(
        DlColor::RGBA(Color::GreenYellow().red, Color::GreenYellow().green,
                      Color::GreenYellow().blue, Color::GreenYellow().alpha));
    builder.DrawCircle({300, 200}, 100, paint);

    paint.setColor(
        DlColor::RGBA(Color::DarkMagenta().red, Color::DarkMagenta().green,
                      Color::DarkMagenta().blue, Color::DarkMagenta().alpha));
    builder.DrawCircle({140, 170}, 75, paint);

    paint.setColor(
        DlColor::RGBA(Color::OrangeRed().red, Color::OrangeRed().green,
                      Color::OrangeRed().blue, Color::OrangeRed().alpha));
    builder.DrawCircle({180, 120}, 100, paint);

    SkRRect rrect =
        SkRRect::MakeRectXY(SkRect::MakeLTRB(a.x, a.y, b.x, b.y), 20, 20);
    builder.ClipRRect(rrect);

    DlPaint save_paint;
    save_paint.setBlendMode(DlBlendMode::kSrc);

    auto backdrop_filter = DlBlurImageFilter::Make(20, 20, DlTileMode::kClamp);
    builder.SaveLayer(nullptr, &save_paint, backdrop_filter.get());
    builder.Restore();

    return builder.Build();
  };

  ASSERT_TRUE(OpenPlaygroundHere(callback));
}

TEST_P(AiksTest, CanRenderBackdropBlur) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kCornflowerBlue());
  builder.DrawCircle({100, 100}, 50, paint);

  paint.setColor(
      DlColor::RGBA(Color::GreenYellow().red, Color::GreenYellow().green,
                    Color::GreenYellow().blue, Color::GreenYellow().alpha));
  builder.DrawCircle({300, 200}, 100, paint);

  paint.setColor(
      DlColor::RGBA(Color::DarkMagenta().red, Color::DarkMagenta().green,
                    Color::DarkMagenta().blue, Color::DarkMagenta().alpha));
  builder.DrawCircle({140, 170}, 75, paint);

  paint.setColor(DlColor::RGBA(Color::OrangeRed().red, Color::OrangeRed().green,
                               Color::OrangeRed().blue,
                               Color::OrangeRed().alpha));
  builder.DrawCircle({180, 120}, 100, paint);

  SkRRect rrect =
      SkRRect::MakeRectXY(SkRect::MakeLTRB(75, 50, 375, 275), 20, 20);
  builder.ClipRRect(rrect);

  DlPaint save_paint;
  save_paint.setBlendMode(DlBlendMode::kSrc);
  auto backdrop_filter = DlBlurImageFilter::Make(30, 30, DlTileMode::kClamp);
  builder.SaveLayer(nullptr, &save_paint, backdrop_filter.get());
  builder.Restore();

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderBackdropBlurHugeSigma) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kGreen());
  builder.DrawCircle({400, 400}, 300, paint);

  DlPaint save_paint;
  save_paint.setBlendMode(DlBlendMode::kSrc);

  auto backdrop_filter =
      DlBlurImageFilter::Make(999999, 999999, DlTileMode::kClamp);
  builder.SaveLayer(nullptr, &save_paint, backdrop_filter.get());
  builder.Restore();

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, CanRenderClippedBlur) {
  DisplayListBuilder builder;
  builder.ClipRect(SkRect::MakeXYWH(100, 150, 400, 400));

  DlPaint paint;
  paint.setColor(DlColor::kGreen());
  paint.setImageFilter(DlBlurImageFilter::Make(20, 20, DlTileMode::kDecal));
  builder.DrawCircle({400, 400}, 200, paint);
  builder.Restore();

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, ClippedBlurFilterRendersCorrectlyInteractive) {
  auto callback = [&]() -> sk_sp<DisplayList> {
    static PlaygroundPoint playground_point(Point(400, 400), 20,
                                            Color::Green());
    auto point = DrawPlaygroundPoint(playground_point);

    DisplayListBuilder builder;
    auto location = point - Point(400, 400);
    builder.Translate(location.x, location.y);

    DlPaint paint;
    paint.setMaskFilter(DlBlurMaskFilter::Make(DlBlurStyle::kNormal,
                                               Sigma(Radius(120 * 3)).sigma));
    paint.setColor(DlColor::kRed());

    SkPath path = SkPath::Rect(SkRect::MakeLTRB(0, 0, 800, 800));
    builder.DrawPath(path, paint);
    return builder.Build();
  };
  ASSERT_TRUE(OpenPlaygroundHere(callback));
}

TEST_P(AiksTest, ClippedBlurFilterRendersCorrectly) {
  DisplayListBuilder builder;
  builder.Translate(0, -400);
  DlPaint paint;
  paint.setMaskFilter(DlBlurMaskFilter::Make(DlBlurStyle::kNormal,
                                             Sigma(Radius(120 * 3)).sigma));
  paint.setColor(DlColor::kRed());

  SkPath path = SkPath::Rect(SkRect::MakeLTRB(0, 0, 800, 800));
  builder.DrawPath(path, paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, ClearBlendWithBlur) {
  DisplayListBuilder builder;
  DlPaint paint;
  paint.setColor(DlColor::kBlue());
  builder.DrawRect(SkRect::MakeXYWH(0, 0, 600.0, 600.0), paint);

  DlPaint clear;
  clear.setBlendMode(DlBlendMode::kClear);
  clear.setMaskFilter(DlBlurMaskFilter::Make(DlBlurStyle::kNormal, 20));

  builder.DrawCircle({300.0, 300.0}, 200.0, clear);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(AiksTest, BlurHasNoEdge) {
  Scalar sigma = 47.6;
  auto callback = [&]() -> sk_sp<DisplayList> {
    if (AiksTest::ImGuiBegin("Controls", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SliderFloat("Sigma", &sigma, 0, 50);
      ImGui::End();
    }
    DisplayListBuilder builder;
    builder.Scale(GetContentScale().x, GetContentScale().y);
    builder.DrawPaint({});

    DlPaint paint;
    paint.setColor(DlColor::kGreen());
    paint.setMaskFilter(DlBlurMaskFilter::Make(DlBlurStyle::kNormal, sigma));

    builder.DrawRect(SkRect::MakeXYWH(300, 300, 200, 200), paint);
    return builder.Build();
  };

  ASSERT_TRUE(OpenPlaygroundHere(callback));
}

TEST_P(AiksTest, MaskBlurWithZeroSigmaIsSkipped) {
  DisplayListBuilder builder;

  DlPaint paint;
  paint.setColor(DlColor::kBlue());
  paint.setMaskFilter(DlBlurMaskFilter::Make(DlBlurStyle::kNormal, 0));

  builder.DrawCircle({300, 300}, 200, paint);
  builder.DrawRect(SkRect::MakeLTRB(100, 300, 500, 600), paint);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

struct MaskBlurTestConfig {
  DlBlurStyle style = DlBlurStyle::kNormal;
  Scalar sigma = 1.0f;
  Scalar alpha = 1.0f;
  std::shared_ptr<DlImageFilter> image_filter;
  bool invert_colors = false;
  DlBlendMode blend_mode = DlBlendMode::kSrcOver;
};

static sk_sp<DisplayList> MaskBlurVariantTest(
    const AiksTest& test_context,
    const MaskBlurTestConfig& config) {
  DisplayListBuilder builder;
  builder.Scale(test_context.GetContentScale().x,
                test_context.GetContentScale().y);
  builder.Scale(0.8f, 0.8f);

  DlPaint draw_paint;
  draw_paint.setColor(
      DlColor::RGBA(Color::AntiqueWhite().red, Color::AntiqueWhite().green,
                    Color::AntiqueWhite().blue, Color::AntiqueWhite().alpha));
  builder.DrawPaint(draw_paint);

  DlPaint paint;
  paint.setMaskFilter(DlBlurMaskFilter::Make(config.style, config.sigma));
  paint.setInvertColors(config.invert_colors);
  paint.setImageFilter(config.image_filter);
  paint.setBlendMode(config.blend_mode);

  const Scalar x = 50;
  const Scalar radius = 20.0f;
  const Scalar y_spacing = 100.0f;
  Scalar alpha = config.alpha * 255;

  Scalar y = 50;
  paint.setColor(DlColor::kCrimson().withAlpha(alpha));
  builder.DrawRect(SkRect::MakeXYWH(x + 25 - radius / 2, y + radius / 2,  //
                                    radius, 60.0f - radius),
                   paint);

  y += y_spacing;
  paint.setColor(DlColor::kBlue().withAlpha(alpha));
  builder.DrawCircle({x + 25, y + 25}, radius, paint);

  y += y_spacing;
  paint.setColor(DlColor::kGreen().withAlpha(alpha));
  builder.DrawOval(SkRect::MakeXYWH(x + 25 - radius / 2, y + radius / 2,  //
                                    radius, 60.0f - radius),
                   paint);

  y += y_spacing;
  paint.setColor(DlColor::kPurple().withAlpha(alpha));
  SkRRect rrect =
      SkRRect::MakeRectXY(SkRect::MakeXYWH(x, y, 60.0f, 60.0f), radius, radius);
  builder.DrawRRect(rrect, paint);

  y += y_spacing;
  paint.setColor(DlColor::kOrange().withAlpha(alpha));

  rrect =
      SkRRect::MakeRectXY(SkRect::MakeXYWH(x, y, 60.0f, 60.0f), radius, 5.0);
  builder.DrawRRect(rrect, paint);

  y += y_spacing;
  paint.setColor(DlColor::kMaroon().withAlpha(alpha));

  {
    SkPath path;
    path.moveTo(x + 0, y + 60);
    path.lineTo(x + 30, y + 0);
    path.lineTo(x + 60, y + 60);
    path.close();

    builder.DrawPath(path, paint);
  }

  y += y_spacing;
  paint.setColor(DlColor::kMaroon().withAlpha(alpha));
  {
    SkPath path;
    path.addArc(SkRect::MakeXYWH(x + 5, y, 50, 50), 90, 180);
    path.addArc(SkRect::MakeXYWH(x + 25, y, 50, 50), 90, 180);
    path.close();
    builder.DrawPath(path, paint);
  }

  return builder.Build();
}

static const std::map<std::string, MaskBlurTestConfig> kPaintVariations = {
    // 1. Normal style, translucent, zero sigma.
    {"NormalTranslucentZeroSigma",
     {.style = DlBlurStyle::kNormal, .sigma = 0.0f, .alpha = 0.5f}},
    // 2. Normal style, translucent.
    {"NormalTranslucent",
     {.style = DlBlurStyle::kNormal, .sigma = 8.0f, .alpha = 0.5f}},
    // 3. Solid style, translucent.
    {"SolidTranslucent",
     {.style = DlBlurStyle::kSolid, .sigma = 8.0f, .alpha = 0.5f}},
    // 4. Solid style, opaque.
    {"SolidOpaque", {.style = DlBlurStyle::kSolid, .sigma = 8.0f}},
    // 5. Solid style, translucent, color & image filtered.
    {"SolidTranslucentWithFilters",
     {.style = DlBlurStyle::kSolid,
      .sigma = 8.0f,
      .alpha = 0.5f,
      .image_filter = DlBlurImageFilter::Make(3, 3, DlTileMode::kClamp),
      .invert_colors = true}},
    // 6. Solid style, translucent, exclusion blended.
    {"SolidTranslucentExclusionBlend",
     {.style = DlBlurStyle::kSolid,
      .sigma = 8.0f,
      .alpha = 0.5f,
      .blend_mode = DlBlendMode::kExclusion}},
    // 7. Inner style, translucent.
    {"InnerTranslucent",
     {.style = DlBlurStyle::kInner, .sigma = 8.0f, .alpha = 0.5f}},
    // 8. Inner style, translucent, blurred.
    {"InnerTranslucentWithBlurImageFilter",
     {.style = DlBlurStyle::kInner,
      .sigma = 8.0f,
      .alpha = 0.5f,
      .image_filter = DlBlurImageFilter::Make(3, 3, DlTileMode::kClamp)}},
    // 9. Outer style, translucent.
    {"OuterTranslucent",
     {.style = DlBlurStyle::kOuter, .sigma = 8.0f, .alpha = 0.5f}},
    // 10. Outer style, opaque, image filtered.
    {"OuterOpaqueWithBlurImageFilter",
     {.style = DlBlurStyle::kOuter,
      .sigma = 8.0f,
      .image_filter = DlBlurImageFilter::Make(3, 3, DlTileMode::kClamp)}},
};

#define MASK_BLUR_VARIANT_TEST(config)                              \
  TEST_P(AiksTest, MaskBlurVariantTest##config) {                   \
    ASSERT_TRUE(OpenPlaygroundHere(                                 \
        MaskBlurVariantTest(*this, kPaintVariations.at(#config)))); \
  }

MASK_BLUR_VARIANT_TEST(NormalTranslucentZeroSigma)
MASK_BLUR_VARIANT_TEST(NormalTranslucent)
MASK_BLUR_VARIANT_TEST(SolidTranslucent)
MASK_BLUR_VARIANT_TEST(SolidOpaque)
MASK_BLUR_VARIANT_TEST(SolidTranslucentWithFilters)
MASK_BLUR_VARIANT_TEST(SolidTranslucentExclusionBlend)
MASK_BLUR_VARIANT_TEST(InnerTranslucent)
MASK_BLUR_VARIANT_TEST(InnerTranslucentWithBlurImageFilter)
MASK_BLUR_VARIANT_TEST(OuterTranslucent)
MASK_BLUR_VARIANT_TEST(OuterOpaqueWithBlurImageFilter)

#undef MASK_BLUR_VARIANT_TEST

}  // namespace testing
}  // namespace impeller
