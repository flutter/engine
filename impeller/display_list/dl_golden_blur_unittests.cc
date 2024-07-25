// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/dl_golden_unittests.h"

#include "flutter/display_list/dl_builder.h"
#include "flutter/display_list/effects/dl_mask_filter.h"
#include "flutter/impeller/golden_tests/screenshot.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "impeller/typographer/backends/skia/text_frame_skia.h"
#include "txt/platform.h"

namespace flutter {
namespace testing {

using impeller::Font;

namespace {
struct TextRenderOptions {
  bool stroke = false;
  SkScalar font_size = 50;
  DlColor color = DlColor::kYellow();
  std::shared_ptr<DlMaskFilter> mask_filter;
};

bool RenderTextInCanvasSkia(DlCanvas* canvas,
                            const std::string& text,
                            const std::string_view& font_fixture,
                            SkPoint position,
                            const TextRenderOptions& options = {}) {
  auto c_font_fixture = std::string(font_fixture);
  auto mapping = flutter::testing::OpenFixtureAsSkData(c_font_fixture.c_str());
  if (!mapping) {
    return false;
  }
  sk_sp<SkFontMgr> font_mgr = txt::GetDefaultFontManager();
  SkFont sk_font(font_mgr->makeFromData(mapping), options.font_size);
  auto blob = SkTextBlob::MakeFromString(text.c_str(), sk_font);
  if (!blob) {
    return false;
  }

  auto frame = impeller::MakeTextFrameFromTextBlobSkia(blob);

  DlPaint text_paint;
  text_paint.setColor(options.color);
  text_paint.setMaskFilter(options.mask_filter);
  // text_paint.mask_blur_descriptor = options.mask_blur_descriptor;
  // text_paint.stroke_width = 1;
  // text_paint.style =
  //     options.stroke ? Paint::Style::kStroke : Paint::Style::kFill;
  canvas->DrawTextFrame(frame, position.x(), position.y(), text_paint);
  return true;
}

}  // namespace

TEST_P(DlGoldenTest, TextBlurMaskFilterRespectCTM) {
  impeller::Point content_scale = GetContentScale();
  auto draw = [&](DlCanvas* canvas,
                  const std::vector<std::unique_ptr<DlImage>>& images) {
    canvas->DrawColor(DlColor(0xff111111));
    canvas->Scale(content_scale.x, content_scale.y);
    canvas->Scale(2, 2);
    TextRenderOptions options;
    options.mask_filter =
        DlBlurMaskFilter::Make(DlBlurStyle::kNormal, /*sigma=*/10,
                               /*respect_ctm=*/true);
    ASSERT_TRUE(RenderTextInCanvasSkia(canvas, "hello world",
                                       "Roboto-Regular.ttf",
                                       SkPoint::Make(101, 101), options));
    options.mask_filter = nullptr;
    options.color = DlColor::kRed();
    ASSERT_TRUE(RenderTextInCanvasSkia(canvas, "hello world",
                                       "Roboto-Regular.ttf",
                                       SkPoint::Make(100, 100), options));
  };

  DisplayListBuilder builder;
  draw(&builder, /*images=*/{});

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(DlGoldenTest, TextBlurMaskFilterDisrespectCTM) {
  impeller::Point content_scale = GetContentScale();
  auto draw = [&](DlCanvas* canvas,
                  const std::vector<std::unique_ptr<DlImage>>& images) {
    canvas->DrawColor(DlColor(0xff111111));
    canvas->Scale(content_scale.x, content_scale.y);
    canvas->Scale(2, 2);
    TextRenderOptions options;
    options.mask_filter =
        DlBlurMaskFilter::Make(DlBlurStyle::kNormal, /*sigma=*/10,
                               /*respect_ctm=*/false);
    ASSERT_TRUE(RenderTextInCanvasSkia(canvas, "hello world",
                                       "Roboto-Regular.ttf",
                                       SkPoint::Make(101, 101), options));
    options.mask_filter = nullptr;
    options.color = DlColor::kRed();
    ASSERT_TRUE(RenderTextInCanvasSkia(canvas, "hello world",
                                       "Roboto-Regular.ttf",
                                       SkPoint::Make(100, 100), options));
  };

  DisplayListBuilder builder;
  draw(&builder, /*images=*/{});

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

namespace {
double CalculateDistance(const uint8_t* left, const uint8_t* right) {
  double diff[4] = {
      static_cast<double>(left[0]) - right[0],  //
      static_cast<double>(left[1]) - right[1],  //
      static_cast<double>(left[2]) - right[2],  //
      static_cast<double>(left[3]) - right[3]   //
  };
  return sqrt((diff[0] * diff[0]) +  //
              (diff[1] * diff[1]) +  //
              (diff[2] * diff[2]) +  //
              (diff[3] * diff[3]));
}

double RMSE(const impeller::testing::Screenshot* left,
            const impeller::testing::Screenshot* right) {
  FML_CHECK(left);
  FML_CHECK(right);
  FML_CHECK(left->GetWidth() == right->GetWidth());
  FML_CHECK(left->GetHeight() == right->GetHeight());

  int64_t samples = left->GetWidth() * left->GetHeight();
  double tally = 0;

  const uint8_t* left_ptr = left->GetBytes();
  const uint8_t* right_ptr = right->GetBytes();
  for (int64_t i = 0; i < samples; ++i, left_ptr += 4, right_ptr += 4) {
    double distance = CalculateDistance(left_ptr, right_ptr);
    tally += distance * distance;
  }

  return sqrt(tally / static_cast<double>(samples));
}
}  // namespace

TEST_P(DlGoldenTest, ShimmerTest) {
  impeller::Point content_scale = GetContentScale();
  auto draw = [&](DlCanvas* canvas, const std::vector<sk_sp<DlImage>>& images,
                  float sigma) {
    canvas->DrawColor(DlColor(0xff111111));
    canvas->Scale(content_scale.x, content_scale.y);

    canvas->Save();
    canvas->Scale(2, 2);
    DlPaint paint;
    canvas->DrawImage(images[0], SkPoint::Make(10.0, 10.0),
                      DlImageSampling::kLinear, &paint);
    canvas->Restore();

    SkRect save_layer_bounds = SkRect::MakeLTRB(0, 0, 1024, 768);
    DlBlurImageFilter blur(sigma, sigma, DlTileMode::kDecal);
    canvas->ClipRect(SkRect::MakeLTRB(11.25, 10, 911.25, 755.3333));
    canvas->SaveLayer(&save_layer_bounds, /*paint=*/nullptr, &blur);
    canvas->Restore();
  };

  std::vector<sk_sp<DlImage>> images;
  images.emplace_back(CreateDlImageForFixture("kalimba.jpg"));

  auto make_screenshot = [&](float sigma) {
    DisplayListBuilder builder;
    draw(&builder, images, sigma);

    std::unique_ptr<impeller::testing::Screenshot> screenshot =
        MakeScreenshot(builder.Build());
    return screenshot;
  };

  float start_sigma = 10.0f;
  std::unique_ptr<impeller::testing::Screenshot> left =
      make_screenshot(start_sigma);
  if (!left) {
    GTEST_SKIP() << "making screenshots not supported.";
  }

  double average_rmse = 0.0;
  const int32_t sample_count = 200;
  for (int i = 1; i <= sample_count; ++i) {
    float sigma = start_sigma + (i / 2.f);
    std::unique_ptr<impeller::testing::Screenshot> right =
        make_screenshot(sigma);
    double rmse = RMSE(left.get(), right.get());
    average_rmse += rmse;

    // std::stringstream ss;
    // ss << "_" << (i - 1);
    // SaveScreenshot(std::move(left), ss.str());
    left = std::move(right);
  }

  average_rmse = average_rmse / sample_count;

  EXPECT_TRUE(average_rmse < 0.86) << "average_rmse: " << average_rmse;
}

}  // namespace testing
}  // namespace flutter
