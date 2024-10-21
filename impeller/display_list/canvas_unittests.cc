// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "display_list/dl_tile_mode.h"
#include "display_list/effects/dl_image_filter.h"
#include "display_list/geometry/dl_geometry_types.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "impeller/display_list/aiks_unittests.h"
#include "impeller/display_list/canvas.h"
#include "impeller/geometry/geometry_asserts.h"

namespace impeller {
namespace testing {

std::unique_ptr<Canvas> CreateTestCanvas(
    ContentContext& context,
    std::optional<Rect> cull_rect = std::nullopt,
    bool requires_readback = false) {
  RenderTarget render_target =
      context.GetRenderTargetCache()->CreateOffscreenMSAA(*context.GetContext(),
                                                          {100, 100}, 1);

  if (cull_rect.has_value()) {
    return std::make_unique<Canvas>(context, render_target, requires_readback,
                                    cull_rect.value());
  }
  return std::make_unique<Canvas>(context, render_target, requires_readback);
}

TEST_P(AiksTest, TransformMultipliesCorrectly) {
  ContentContext context(GetContext(), nullptr);
  auto canvas = CreateTestCanvas(context);

  ASSERT_MATRIX_NEAR(canvas->GetCurrentTransform(), Matrix());

  // clang-format off
  canvas->Translate(Vector3(100, 200));
  ASSERT_MATRIX_NEAR(
    canvas->GetCurrentTransform(),
    Matrix(  1,   0,   0,   0,
             0,   1,   0,   0,
             0,   0,   1,   0,
           100, 200,   0,   1));

  canvas->Rotate(Radians(kPiOver2));
  ASSERT_MATRIX_NEAR(
    canvas->GetCurrentTransform(),
    Matrix(  0,   1,   0,   0,
            -1,   0,   0,   0,
             0,   0,   1,   0,
           100, 200,   0,   1));

  canvas->Scale(Vector3(2, 3));
  ASSERT_MATRIX_NEAR(
    canvas->GetCurrentTransform(),
    Matrix(  0,   2,   0,   0,
            -3,   0,   0,   0,
             0,   0,   0,   0,
           100, 200,   0,   1));

  canvas->Translate(Vector3(100, 200));
  ASSERT_MATRIX_NEAR(
    canvas->GetCurrentTransform(),
    Matrix(   0,   2,   0,   0,
             -3,   0,   0,   0,
              0,   0,   0,   0,
           -500, 400,   0,   1));
  // clang-format on
}

TEST_P(AiksTest, CanvasCanPushPopCTM) {
  ContentContext context(GetContext(), nullptr);
  auto canvas = CreateTestCanvas(context);

  ASSERT_EQ(canvas->GetSaveCount(), 1u);
  ASSERT_EQ(canvas->Restore(), false);

  canvas->Translate(Size{100, 100});
  canvas->Save(10);
  ASSERT_EQ(canvas->GetSaveCount(), 2u);
  ASSERT_MATRIX_NEAR(canvas->GetCurrentTransform(),
                     Matrix::MakeTranslation({100.0, 100.0, 0.0}));
  ASSERT_TRUE(canvas->Restore());
  ASSERT_EQ(canvas->GetSaveCount(), 1u);
  ASSERT_MATRIX_NEAR(canvas->GetCurrentTransform(),
                     Matrix::MakeTranslation({100.0, 100.0, 0.0}));
}

TEST_P(AiksTest, CanvasCTMCanBeUpdated) {
  ContentContext context(GetContext(), nullptr);
  auto canvas = CreateTestCanvas(context);

  Matrix identity;
  ASSERT_MATRIX_NEAR(canvas->GetCurrentTransform(), identity);
  canvas->Translate(Size{100, 100});
  ASSERT_MATRIX_NEAR(canvas->GetCurrentTransform(),
                     Matrix::MakeTranslation({100.0, 100.0, 0.0}));
}

TEST_P(AiksTest, BackdropCountDownNormal) {
  ContentContext context(GetContext(), nullptr);
  if (!context.GetDeviceCapabilities().SupportsFramebufferFetch()) {
    GTEST_SKIP() << "Test requires device with framebuffer fetch";
  }
  auto canvas = CreateTestCanvas(context, Rect::MakeLTRB(0, 0, 100, 100),
                                 /*requires_readback=*/true);
  // 3 backdrop filters
  canvas->SetBackdropData({}, 3);

  auto blur =
      flutter::DlBlurImageFilter::Make(4, 4, flutter::DlTileMode::kClamp);

  EXPECT_TRUE(canvas->RequiresReadback());
  canvas->DrawRect(flutter::DlRect::MakeLTRB(0, 0, 50, 50),
                   {.color = Color::Azure()});
  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents);
  canvas->Restore();
  EXPECT_TRUE(canvas->RequiresReadback());

  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents);
  canvas->Restore();
  EXPECT_TRUE(canvas->RequiresReadback());

  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents);
  canvas->Restore();
  EXPECT_FALSE(canvas->RequiresReadback());
}

TEST_P(AiksTest, BackdropCountDownBackdropId) {
  ContentContext context(GetContext(), nullptr);
  if (!context.GetDeviceCapabilities().SupportsFramebufferFetch()) {
    GTEST_SKIP() << "Test requires device with framebuffer fetch";
  }
  auto canvas = CreateTestCanvas(context, Rect::MakeLTRB(0, 0, 100, 100),
                                 /*requires_readback=*/true);
  // 3 backdrop filters all with same id.
  std::unordered_map<int64_t, BackdropData> data;
  data[1] = BackdropData{.backdrop_count = 3};
  canvas->SetBackdropData(data, 3);

  auto blur =
      flutter::DlBlurImageFilter::Make(4, 4, flutter::DlTileMode::kClamp);

  EXPECT_TRUE(canvas->RequiresReadback());
  canvas->DrawRect(flutter::DlRect::MakeLTRB(0, 0, 50, 50),
                   {.color = Color::Azure()});
  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents,
                    /*total_content_depth=*/1, /*can_distribute_opacity=*/false,
                    /*backdrop_id=*/1);
  canvas->Restore();
  EXPECT_FALSE(canvas->RequiresReadback());

  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents,
                    /*total_content_depth=*/1, /*can_distribute_opacity=*/false,
                    /*backdrop_id=*/1);
  canvas->Restore();
  EXPECT_FALSE(canvas->RequiresReadback());

  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents,
                    /*total_content_depth=*/1, /*can_distribute_opacity=*/false,
                    /*backdrop_id=*/1);
  canvas->Restore();
  EXPECT_FALSE(canvas->RequiresReadback());
}

TEST_P(AiksTest, BackdropCountDownBackdropIdMixed) {
  ContentContext context(GetContext(), nullptr);
  if (!context.GetDeviceCapabilities().SupportsFramebufferFetch()) {
    GTEST_SKIP() << "Test requires device with framebuffer fetch";
  }
  auto canvas = CreateTestCanvas(context, Rect::MakeLTRB(0, 0, 100, 100),
                                 /*requires_readback=*/true);
  // 3 backdrop filters, 2 with same id.
  std::unordered_map<int64_t, BackdropData> data;
  data[1] = BackdropData{.backdrop_count = 2};
  canvas->SetBackdropData(data, 3);

  auto blur =
      flutter::DlBlurImageFilter::Make(4, 4, flutter::DlTileMode::kClamp);

  EXPECT_TRUE(canvas->RequiresReadback());
  canvas->DrawRect(flutter::DlRect::MakeLTRB(0, 0, 50, 50),
                   {.color = Color::Azure()});
  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents, 1, false);
  canvas->Restore();
  EXPECT_TRUE(canvas->RequiresReadback());

  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents, 1, false, 1);
  canvas->Restore();
  EXPECT_FALSE(canvas->RequiresReadback());

  canvas->SaveLayer({}, std::nullopt, blur.get(),
                    ContentBoundsPromise::kContainsContents, 1, false, 1);
  canvas->Restore();
  EXPECT_FALSE(canvas->RequiresReadback());
}

}  // namespace testing
}  // namespace impeller
