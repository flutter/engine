// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/effects/dl_color_source.h"
#include "flutter/display_list/effects/dl_runtime_effect.h"
#include "flutter/display_list/image/dl_image.h"
#include "flutter/display_list/testing/dl_test_equality.h"

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {
namespace testing {

static sk_sp<DlImage> MakeTestImage(int w, int h, SkColor color) {
  sk_sp<SkSurface> surface;
  if (SkColorGetA(color) < 255) {
    surface = SkSurface::MakeRasterN32Premul(w, h);
  } else {
    SkImageInfo info =
        SkImageInfo::MakeN32(w, h, SkAlphaType::kOpaque_SkAlphaType);
    surface = SkSurface::MakeRaster(info);
  }
  SkCanvas* canvas = surface->getCanvas();
  canvas->drawColor(color);
  return DlImage::Make(surface->makeImageSnapshot());
}

static const auto kTestRuntimeEffect1 = DlRuntimeEffect::MakeSkia(
    SkRuntimeEffect::MakeForShader(
        SkString("vec4 main(vec2 p) { return vec4(0); }"))
        .effect);
static const auto kTestRuntimeEffect2 = DlRuntimeEffect::MakeSkia(
    SkRuntimeEffect::MakeForShader(
        SkString("vec4 main(vec2 p) { return vec4(1); }"))
        .effect);

static const sk_sp<DlImage> kTestImage1 = MakeTestImage(10, 10, SK_ColorGREEN);
static const sk_sp<DlImage> kTestAlphaImage1 =
    MakeTestImage(10, 10, SK_ColorTRANSPARENT);
// clang-format off
static const SkMatrix kTestMatrix1 =
    SkMatrix::MakeAll(2, 0, 10,
                      0, 3, 12,
                      0, 0, 1);
static const SkMatrix kTestMatrix2 =
    SkMatrix::MakeAll(4, 0, 15,
                      0, 7, 17,
                      0, 0, 1);
// clang-format on
static constexpr int kTestStopCount = 3;
static constexpr DlColor kTestColors[kTestStopCount] = {
    DlColor::kRed(),
    DlColor::kGreen(),
    DlColor::kBlue(),
};
static const DlColor kTestAlphaColors[kTestStopCount] = {
    DlColor::kBlue().withAlpha(0x7F),
    DlColor::kRed().withAlpha(0x2F),
    DlColor::kGreen().withAlpha(0xCF),
};
static constexpr float kTestStops[kTestStopCount] = {
    0.0f,
    0.7f,
    1.0f,
};
static constexpr float kTestStops2[kTestStopCount] = {
    0.0f,
    0.3f,
    1.0f,
};
static constexpr SkPoint kTestPoints[2] = {
    SkPoint::Make(5, 15),
    SkPoint::Make(7, 18),
};
static constexpr SkPoint kTestPoints2[2] = {
    SkPoint::Make(100, 115),
    SkPoint::Make(107, 118),
};

TEST(DisplayListColorSource, ColorConstructor) {
  auto source = DlColorSource::MakeColor(SK_ColorRED);
}

TEST(DisplayListColorSource, ColorAsColor) {
  auto source = DlColorSource::MakeColor(SK_ColorRED);
  ASSERT_NE(source->asColor(), nullptr);
  ASSERT_EQ(source->asColor(), source.get());

  ASSERT_EQ(source->asImage(), nullptr);
  ASSERT_EQ(source->asLinearGradient(), nullptr);
  ASSERT_EQ(source->asRadialGradient(), nullptr);
  ASSERT_EQ(source->asConicalGradient(), nullptr);
  ASSERT_EQ(source->asSweepGradient(), nullptr);
  ASSERT_EQ(source->asRuntimeEffect(), nullptr);
}

TEST(DisplayListColorSource, ColorContents) {
  auto source = DlColorColorSource::Make(SK_ColorRED);
  ASSERT_EQ(source->color(), SK_ColorRED);
  ASSERT_EQ(source->is_opaque(), true);
  for (int i = 0; i < 255; i++) {
    SkColor alpha_color = SkColorSetA(SK_ColorRED, i);
    auto alpha_source = DlColorColorSource::Make(alpha_color);
    ASSERT_EQ(alpha_source->color(), alpha_color);
    ASSERT_EQ(alpha_source->is_opaque(), false);
  }
}

TEST(DisplayListColorSource, ColorEquals) {
  auto source1 = DlColorSource::MakeColor(SK_ColorRED);
  auto source2 = DlColorSource::MakeColor(SK_ColorRED);
  TestEquals(*source1, *source2);
}

TEST(DisplayListColorSource, ColorNotEquals) {
  auto source1 = DlColorSource::MakeColor(SK_ColorRED);
  auto source2 = DlColorSource::MakeColor(SK_ColorBLUE);
  TestNotEquals(*source1, *source2, "Color differs");
}

TEST(DisplayListColorSource, ImageConstructor) {
  auto source = DlColorSource::MakeImage(
      kTestImage1, DlTileMode::kClamp, DlTileMode::kClamp,
      DlImageSampling::kLinear, &kTestMatrix1);
}

TEST(DisplayListColorSource, ImageAsImage) {
  auto source = DlColorSource::MakeImage(
      kTestImage1, DlTileMode::kClamp, DlTileMode::kClamp,
      DlImageSampling::kLinear, &kTestMatrix1);
  ASSERT_NE(source->asImage(), nullptr);
  ASSERT_EQ(source->asImage(), source.get());

  ASSERT_EQ(source->asColor(), nullptr);
  ASSERT_EQ(source->asLinearGradient(), nullptr);
  ASSERT_EQ(source->asRadialGradient(), nullptr);
  ASSERT_EQ(source->asConicalGradient(), nullptr);
  ASSERT_EQ(source->asSweepGradient(), nullptr);
}

TEST(DisplayListColorSource, ImageContents) {
  auto source = DlImageColorSource::Make(
      kTestImage1, DlTileMode::kRepeat, DlTileMode::kMirror,
      DlImageSampling::kLinear, &kTestMatrix1);
  ASSERT_EQ(source->image(), kTestImage1);
  ASSERT_EQ(source->horizontal_tile_mode(), DlTileMode::kRepeat);
  ASSERT_EQ(source->vertical_tile_mode(), DlTileMode::kMirror);
  ASSERT_EQ(source->sampling(), DlImageSampling::kLinear);
  ASSERT_EQ(source->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), true);
}

TEST(DisplayListColorSource, AlphaImageContents) {
  auto source = DlImageColorSource::Make(
      kTestAlphaImage1, DlTileMode::kRepeat, DlTileMode::kMirror,
      DlImageSampling::kLinear, &kTestMatrix1);
  ASSERT_EQ(source->image(), kTestAlphaImage1);
  ASSERT_EQ(source->horizontal_tile_mode(), DlTileMode::kRepeat);
  ASSERT_EQ(source->vertical_tile_mode(), DlTileMode::kMirror);
  ASSERT_EQ(source->sampling(), DlImageSampling::kLinear);
  ASSERT_EQ(source->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), false);
}

TEST(DisplayListColorSource, ImageEquals) {
  auto source1 = DlColorSource::MakeImage(
      kTestImage1, DlTileMode::kClamp, DlTileMode::kMirror,
      DlImageSampling::kLinear, &kTestMatrix1);
  auto source2 = DlColorSource::MakeImage(
      kTestImage1, DlTileMode::kClamp, DlTileMode::kMirror,
      DlImageSampling::kLinear, &kTestMatrix1);
  TestEquals(*source1, *source2);
}

TEST(DisplayListColorSource, ImageNotEquals) {
  auto source1 = DlColorSource::MakeImage(
      kTestImage1, DlTileMode::kClamp, DlTileMode::kMirror,
      DlImageSampling::kLinear, &kTestMatrix1);
  {
    auto source2 = DlColorSource::MakeImage(
        kTestAlphaImage1, DlTileMode::kClamp, DlTileMode::kMirror,
        DlImageSampling::kLinear, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Image differs");
  }
  {
    auto source2 = DlColorSource::MakeImage(
        kTestImage1, DlTileMode::kRepeat, DlTileMode::kMirror,
        DlImageSampling::kLinear, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "hTileMode differs");
  }
  {
    auto source2 = DlColorSource::MakeImage(
        kTestImage1, DlTileMode::kClamp, DlTileMode::kRepeat,
        DlImageSampling::kLinear, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "vTileMode differs");
  }
  {
    auto source2 = DlColorSource::MakeImage(
        kTestImage1, DlTileMode::kClamp, DlTileMode::kMirror,
        DlImageSampling::kCubic, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Sampling differs");
  }
  {
    auto source2 = DlColorSource::MakeImage(
        kTestImage1, DlTileMode::kClamp, DlTileMode::kMirror,
        DlImageSampling::kLinear, &kTestMatrix2);
    TestNotEquals(*source1, *source2, "Matrix differs");
  }
}

TEST(DisplayListColorSource, LinearGradientConstructor) {
  auto source = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
}

TEST(DisplayListColorSource, LinearGradientAsLinear) {
  auto source = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_NE(source->asLinearGradient(), nullptr);
  ASSERT_EQ(source->asLinearGradient(), source.get());

  ASSERT_EQ(source->asColor(), nullptr);
  ASSERT_EQ(source->asImage(), nullptr);
  ASSERT_EQ(source->asRadialGradient(), nullptr);
  ASSERT_EQ(source->asConicalGradient(), nullptr);
  ASSERT_EQ(source->asSweepGradient(), nullptr);
  ASSERT_EQ(source->asRuntimeEffect(), nullptr);
}

TEST(DisplayListColorSource, LinearGradientContents) {
  auto source = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asLinearGradient()->start_point(), kTestPoints[0]);
  ASSERT_EQ(source->asLinearGradient()->end_point(), kTestPoints[1]);
  ASSERT_EQ(source->asLinearGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asLinearGradient()->colors()[i], kTestColors[i]);
    ASSERT_EQ(source->asLinearGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asLinearGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asLinearGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), true);
}

TEST(DisplayListColorSource, AlphaLinearGradientContents) {
  auto source = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestAlphaColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asLinearGradient()->start_point(), kTestPoints[0]);
  ASSERT_EQ(source->asLinearGradient()->end_point(), kTestPoints[1]);
  ASSERT_EQ(source->asLinearGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asLinearGradient()->colors()[i], kTestAlphaColors[i]);
    ASSERT_EQ(source->asLinearGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asLinearGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asLinearGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), false);
}

TEST(DisplayListColorSource, LinearGradientEquals) {
  auto source1 = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  auto source2 = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  TestEquals(*source1, *source2);
}

TEST(DisplayListColorSource, LinearGradientNotEquals) {
  auto source1 = DlColorSource::MakeLinear(
      kTestPoints[0], kTestPoints[1], kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  {
    auto source2 =  //
        DlColorSource::MakeLinear(kTestPoints2[0], kTestPoints[1],
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Point 0 differs");
  }
  {
    auto source2 =  //
        DlColorSource::MakeLinear(kTestPoints[0], kTestPoints2[1],
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Point 1 differs");
  }
  {
    auto source2 =                                                 //
        DlColorSource::MakeLinear(kTestPoints[0], kTestPoints[1],  //
                                  2, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stop count differs");
  }
  {
    auto source2 =  //
        DlColorSource::MakeLinear(kTestPoints[0], kTestPoints[1],
                                  kTestStopCount, kTestAlphaColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Colors differ");
  }
  {
    auto source2 =  //
        DlColorSource::MakeLinear(kTestPoints[0], kTestPoints[1],
                                  kTestStopCount, kTestColors, kTestStops2,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stops differ");
  }
  {
    auto source2 =  //
        DlColorSource::MakeLinear(kTestPoints[0], kTestPoints[1],
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kMirror, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Tile Mode differs");
  }
  {
    auto source2 =  //
        DlColorSource::MakeLinear(kTestPoints[0], kTestPoints[1],
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix2);
    TestNotEquals(*source1, *source2, "Matrix differs");
  }
}

TEST(DisplayListColorSource, RadialGradientConstructor) {
  auto source =                                        //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
}

TEST(DisplayListColorSource, RadialGradientAsRadial) {
  auto source =                                        //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_NE(source->asRadialGradient(), nullptr);
  ASSERT_EQ(source->asRadialGradient(), source.get());

  ASSERT_EQ(source->asColor(), nullptr);
  ASSERT_EQ(source->asImage(), nullptr);
  ASSERT_EQ(source->asLinearGradient(), nullptr);
  ASSERT_EQ(source->asConicalGradient(), nullptr);
  ASSERT_EQ(source->asSweepGradient(), nullptr);
  ASSERT_EQ(source->asRuntimeEffect(), nullptr);
}

TEST(DisplayListColorSource, RadialGradientContents) {
  auto source =                                        //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asRadialGradient()->center(), kTestPoints[0]);
  ASSERT_EQ(source->asRadialGradient()->radius(), 10.0);
  ASSERT_EQ(source->asRadialGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asRadialGradient()->colors()[i], kTestColors[i]);
    ASSERT_EQ(source->asRadialGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asRadialGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asRadialGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), true);
}

TEST(DisplayListColorSource, AlphaRadialGradientContents) {
  auto source =                                        //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestAlphaColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asRadialGradient()->center(), kTestPoints[0]);
  ASSERT_EQ(source->asRadialGradient()->radius(), 10.0);
  ASSERT_EQ(source->asRadialGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asRadialGradient()->colors()[i], kTestAlphaColors[i]);
    ASSERT_EQ(source->asRadialGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asRadialGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asRadialGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), false);
}

TEST(DisplayListColorSource, RadialGradientEquals) {
  auto source1 =                                       //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
  auto source2 =                                       //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
  TestEquals(*source1, *source2);
}

TEST(DisplayListColorSource, RadialGradientNotEquals) {
  auto source1 =                                       //
      DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                kTestStopCount, kTestColors, kTestStops,
                                DlTileMode::kClamp, &kTestMatrix1);
  {
    auto source2 =                                        //
        DlColorSource::MakeRadial(kTestPoints2[0], 10.0,  //
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Center differs");
  }
  {
    auto source2 =                                       //
        DlColorSource::MakeRadial(kTestPoints[0], 20.0,  //
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Radius differs");
  }
  {
    auto source2 =                                       //
        DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                  2, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stop count differs");
  }
  {
    auto source2 =                                       //
        DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                  kTestStopCount, kTestAlphaColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Colors differ");
  }
  {
    auto source2 =                                       //
        DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                  kTestStopCount, kTestColors, kTestStops2,
                                  DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stops differ");
  }
  {
    auto source2 =                                       //
        DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kMirror, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Tile Mode differs");
  }
  {
    auto source2 =                                       //
        DlColorSource::MakeRadial(kTestPoints[0], 10.0,  //
                                  kTestStopCount, kTestColors, kTestStops,
                                  DlTileMode::kClamp, &kTestMatrix2);
    TestNotEquals(*source1, *source2, "Matrix differs");
  }
}

TEST(DisplayListColorSource, ConicalGradientConstructor) {
  auto source = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
}

TEST(DisplayListColorSource, ConicalGradientAsConical) {
  auto source = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_NE(source->asConicalGradient(), nullptr);
  ASSERT_EQ(source->asConicalGradient(), source.get());

  ASSERT_EQ(source->asColor(), nullptr);
  ASSERT_EQ(source->asImage(), nullptr);
  ASSERT_EQ(source->asLinearGradient(), nullptr);
  ASSERT_EQ(source->asRadialGradient(), nullptr);
  ASSERT_EQ(source->asSweepGradient(), nullptr);
  ASSERT_EQ(source->asRuntimeEffect(), nullptr);
}

TEST(DisplayListColorSource, ConicalGradientContents) {
  auto source = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asConicalGradient()->start_center(), kTestPoints[0]);
  ASSERT_EQ(source->asConicalGradient()->start_radius(), 10.0);
  ASSERT_EQ(source->asConicalGradient()->end_center(), kTestPoints[1]);
  ASSERT_EQ(source->asConicalGradient()->end_radius(), 20.0);
  ASSERT_EQ(source->asConicalGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asConicalGradient()->colors()[i], kTestColors[i]);
    ASSERT_EQ(source->asConicalGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asConicalGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asConicalGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), true);
}

TEST(DisplayListColorSource, AlphaConicalGradientContents) {
  auto source = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount,
      kTestAlphaColors, kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asConicalGradient()->start_center(), kTestPoints[0]);
  ASSERT_EQ(source->asConicalGradient()->start_radius(), 10.0);
  ASSERT_EQ(source->asConicalGradient()->end_center(), kTestPoints[1]);
  ASSERT_EQ(source->asConicalGradient()->end_radius(), 20.0);
  ASSERT_EQ(source->asConicalGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asConicalGradient()->colors()[i], kTestAlphaColors[i]);
    ASSERT_EQ(source->asConicalGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asConicalGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asConicalGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), false);
}

TEST(DisplayListColorSource, ConicalGradientEquals) {
  auto source1 = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  auto source2 = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  TestEquals(*source1, *source2);
}

TEST(DisplayListColorSource, ConicalGradientNotEquals) {
  auto source1 = DlColorSource::MakeConical(
      kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
      kTestStops, DlTileMode::kClamp, &kTestMatrix1);
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints2[0], 10.0, kTestPoints[1], 20.0, kTestStopCount,
        kTestColors, kTestStops, DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Start Center differs");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 15.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
        kTestStops, DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Start Radius differs");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints2[1], 20.0, kTestStopCount,
        kTestColors, kTestStops, DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "End Center differs");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints[1], 25.0, kTestStopCount, kTestColors,
        kTestStops, DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "End Radius differs");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints[1], 20.0, 2, kTestColors, kTestStops,
        DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stop count differs");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount,
        kTestAlphaColors, kTestStops, DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Colors differ");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
        kTestStops2, DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stops differ");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
        kTestStops, DlTileMode::kMirror, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Tile Mode differs");
  }
  {
    auto source2 = DlColorSource::MakeConical(
        kTestPoints[0], 10.0, kTestPoints[1], 20.0, kTestStopCount, kTestColors,
        kTestStops, DlTileMode::kClamp, &kTestMatrix2);
    TestNotEquals(*source1, *source2, "Matrix differs");
  }
}

TEST(DisplayListColorSource, SweepGradientConstructor) {
  auto source = DlColorSource::MakeSweep(
      kTestPoints[0], 10.0, 20.0, kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
}

TEST(DisplayListColorSource, SweepGradientAsSweep) {
  auto source = DlColorSource::MakeSweep(
      kTestPoints[0], 10.0, 20.0, kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_NE(source->asSweepGradient(), nullptr);
  ASSERT_EQ(source->asSweepGradient(), source.get());

  ASSERT_EQ(source->asColor(), nullptr);
  ASSERT_EQ(source->asImage(), nullptr);
  ASSERT_EQ(source->asLinearGradient(), nullptr);
  ASSERT_EQ(source->asRadialGradient(), nullptr);
  ASSERT_EQ(source->asConicalGradient(), nullptr);
  ASSERT_EQ(source->asRuntimeEffect(), nullptr);
}

TEST(DisplayListColorSource, SweepGradientContents) {
  auto source = DlColorSource::MakeSweep(
      kTestPoints[0], 10.0, 20.0, kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asSweepGradient()->center(), kTestPoints[0]);
  ASSERT_EQ(source->asSweepGradient()->start(), 10.0);
  ASSERT_EQ(source->asSweepGradient()->end(), 20.0);
  ASSERT_EQ(source->asSweepGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asSweepGradient()->colors()[i], kTestColors[i]);
    ASSERT_EQ(source->asSweepGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asSweepGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asSweepGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), true);
}

TEST(DisplayListColorSource, AlphaSweepGradientContents) {
  auto source = DlColorSource::MakeSweep(
      kTestPoints[0], 10.0, 20.0, kTestStopCount, kTestAlphaColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  ASSERT_EQ(source->asSweepGradient()->center(), kTestPoints[0]);
  ASSERT_EQ(source->asSweepGradient()->start(), 10.0);
  ASSERT_EQ(source->asSweepGradient()->end(), 20.0);
  ASSERT_EQ(source->asSweepGradient()->stop_count(), kTestStopCount);
  for (int i = 0; i < kTestStopCount; i++) {
    ASSERT_EQ(source->asSweepGradient()->colors()[i], kTestAlphaColors[i]);
    ASSERT_EQ(source->asSweepGradient()->stops()[i], kTestStops[i]);
  }
  ASSERT_EQ(source->asSweepGradient()->tile_mode(), DlTileMode::kClamp);
  ASSERT_EQ(source->asSweepGradient()->matrix(), kTestMatrix1);
  ASSERT_EQ(source->is_opaque(), false);
}

TEST(DisplayListColorSource, SweepGradientEquals) {
  auto source1 = DlColorSource::MakeSweep(
      kTestPoints[0], 10.0, 20.0, kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  auto source2 = DlColorSource::MakeSweep(
      kTestPoints[0], 10.0, 20.0, kTestStopCount, kTestColors, kTestStops,
      DlTileMode::kClamp, &kTestMatrix1);
  TestEquals(*source1, *source2);
}

TEST(DisplayListColorSource, SweepGradientNotEquals) {
  auto source1 =                                            //
      DlColorSource::MakeSweep(kTestPoints[0], 10.0, 20.0,  //
                               kTestStopCount, kTestColors, kTestStops,
                               DlTileMode::kClamp, &kTestMatrix1);
  {
    auto source2 =                                             //
        DlColorSource::MakeSweep(kTestPoints2[0], 10.0, 20.0,  //
                                 kTestStopCount, kTestColors, kTestStops,
                                 DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Center differs");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 15.0, 20.0,  //
                                 kTestStopCount, kTestColors, kTestStops,
                                 DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Start Angle differs");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 10.0, 25.0,  //
                                 kTestStopCount, kTestColors, kTestStops,
                                 DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "End Angle differs");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 10.0, 20.0,  //
                                 2, kTestColors, kTestStops,  //
                                 DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stop count differs");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 10.0, 20.0,  //
                                 kTestStopCount, kTestAlphaColors, kTestStops,
                                 DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Colors differ");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 10.0, 20.0,  //
                                 kTestStopCount, kTestColors, kTestStops2,
                                 DlTileMode::kClamp, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Stops differ");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 10.0, 20.0,  //
                                 kTestStopCount, kTestColors, kTestStops,
                                 DlTileMode::kMirror, &kTestMatrix1);
    TestNotEquals(*source1, *source2, "Tile Mode differs");
  }
  {
    auto source2 =                                            //
        DlColorSource::MakeSweep(kTestPoints[0], 10.0, 20.0,  //
                                 kTestStopCount, kTestColors, kTestStops,
                                 DlTileMode::kClamp, &kTestMatrix2);
    TestNotEquals(*source1, *source2, "Matrix differs");
  }
}

TEST(DisplayListColorSource, RuntimeEffect) {
  auto source1 = DlColorSource::MakeRuntimeEffect(
      kTestRuntimeEffect1, {}, std::make_shared<std::vector<uint8_t>>());
  auto source2 = DlColorSource::MakeRuntimeEffect(
      kTestRuntimeEffect2, {}, std::make_shared<std::vector<uint8_t>>());
  auto source3 = DlColorSource::MakeRuntimeEffect(
      nullptr, {}, std::make_shared<std::vector<uint8_t>>());

  ASSERT_EQ(source1->type(), DlColorSourceType::kRuntimeEffect);
  ASSERT_EQ(source1->asRuntimeEffect(), source1.get());
  ASSERT_NE(source2->asRuntimeEffect(), source1.get());

  ASSERT_EQ(source1->asImage(), nullptr);
  ASSERT_EQ(source1->asColor(), nullptr);
  ASSERT_EQ(source1->asLinearGradient(), nullptr);
  ASSERT_EQ(source1->asRadialGradient(), nullptr);
  ASSERT_EQ(source1->asConicalGradient(), nullptr);
  ASSERT_EQ(source1->asSweepGradient(), nullptr);

  TestEquals(*source1, *source1);
  TestEquals(*source3, *source3);
  TestNotEquals(*source1, *source2, "SkRuntimeEffect differs");
  TestNotEquals(*source2, *source3, "SkRuntimeEffect differs");
}

}  // namespace testing
}  // namespace flutter
