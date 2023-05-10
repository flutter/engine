// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/testing/mock_texture.h"

#include "flutter/display_list/dl_builder.h"
#include "flutter/testing/display_list_testing.h"
#include "flutter/testing/mock_canvas.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(MockTextureTest, Callbacks) {
  auto texture = std::make_shared<MockTexture>(0);

  ASSERT_FALSE(texture->gr_context_created());
  texture->OnGrContextCreated();
  ASSERT_TRUE(texture->gr_context_created());

  ASSERT_FALSE(texture->gr_context_destroyed());
  texture->OnGrContextDestroyed();
  ASSERT_TRUE(texture->gr_context_destroyed());

  ASSERT_FALSE(texture->unregistered());
  texture->OnTextureUnregistered();
  ASSERT_TRUE(texture->unregistered());
}

TEST(MockTextureTest, PaintCalls) {
  MockCanvas canvas;
  DisplayListBuilder builder;
  const SkRect paint_bounds1 = SkRect::MakeWH(1.0f, 1.0f);
  const SkRect paint_bounds2 = SkRect::MakeWH(2.0f, 2.0f);
  const DlImageSampling sampling = DlImageSampling::kNearestNeighbor;
  auto texture = std::make_shared<MockTexture>(0);
  DlPaint paint1 = DlPaint(texture->mockColor(0xff, false, sampling));
  DlPaint paint2 = DlPaint(texture->mockColor(0xff, true, sampling));

  Texture::PaintContext context{
      .canvas = &builder,
  };
  texture->Paint(context, paint_bounds1, false, sampling);
  texture->Paint(context, paint_bounds2, true, sampling);

  DisplayListBuilder expected_builder;
  expected_builder.DrawRect(paint_bounds1, paint1);
  expected_builder.DrawRect(paint_bounds2, paint2);
  EXPECT_TRUE(
      DisplayListsEQ_Verbose(builder.Build(), expected_builder.Build()));
}

TEST(MockTextureTest, PaintCallsWithLinearSampling) {
  MockCanvas canvas;
  DisplayListBuilder builder;
  const SkRect paint_bounds1 = SkRect::MakeWH(1.0f, 1.0f);
  const SkRect paint_bounds2 = SkRect::MakeWH(2.0f, 2.0f);
  const auto sampling = DlImageSampling::kLinear;
  auto texture = std::make_shared<MockTexture>(0);
  DlPaint paint1 = DlPaint(texture->mockColor(0xff, false, sampling));
  DlPaint paint2 = DlPaint(texture->mockColor(0xff, true, sampling));

  Texture::PaintContext context{
      .canvas = &builder,
  };
  texture->Paint(context, paint_bounds1, false, sampling);
  texture->Paint(context, paint_bounds2, true, sampling);

  DisplayListBuilder expected_builder;
  expected_builder.DrawRect(paint_bounds1, paint1);
  expected_builder.DrawRect(paint_bounds2, paint2);
  EXPECT_TRUE(
      DisplayListsEQ_Verbose(builder.Build(), expected_builder.Build()));
}

}  // namespace testing
}  // namespace flutter
