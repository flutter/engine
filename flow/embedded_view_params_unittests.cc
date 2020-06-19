// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"
#include "gtest/gtest.h"
#include "flutter/fml/logging.h"


namespace flutter {
namespace testing {

TEST(EmbeddedViewParams, GetBoundingRectWithNoMutations) {
  MutatorsStack stack;
  EmbeddedViewParams params;
  params.sizePoints = SkSize::Make(1, 1);;
  params.offsetPixels = SkPoint::Make(0, 0);
  params.mutatorsStack = stack;

  SkRect rect = params.GetBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 1));
}

TEST(EmbeddedViewParams, GetBoundingRectWithScale) {
  MutatorsStack stack;
  SkMatrix scale = SkMatrix::Scale(2, 2);
  stack.PushTransform(scale);

  EmbeddedViewParams params;
  params.sizePoints = SkSize::Make(1, 1);;
  params.offsetPixels = SkPoint::Make(0, 0);
  params.mutatorsStack = stack;

  SkRect rect = params.GetBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 2));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 2));
}

TEST(EmbeddedViewParams, GetBoundingRectWithTranslate) {
  MutatorsStack stack;
  SkMatrix trans = SkMatrix::MakeTrans(1, 1);
  stack.PushTransform(trans);

  EmbeddedViewParams params;
  params.sizePoints = SkSize::Make(1, 1);;
  params.offsetPixels = SkPoint::Make(0, 0);
  params.mutatorsStack = stack;

  SkRect rect = params.GetBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 1));
}

TEST(EmbeddedViewParams, GetBoundingRectWithRotation90) {
  MutatorsStack stack;
  SkMatrix rotate;
  rotate.setRotate(90);
  stack.PushTransform(rotate);

  EmbeddedViewParams params;
  params.sizePoints = SkSize::Make(1, 1);;
  params.offsetPixels = SkPoint::Make(0, 0);
  params.mutatorsStack = stack;

  SkRect rect = params.GetBoundingRect();
  FML_DLOG(ERROR) << rect.x();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), -1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 1));
}

TEST(EmbeddedViewParams, GetBoundingRectWithRotation45) {
  MutatorsStack stack;
  SkMatrix rotate;
  rotate.setRotate(45);
  stack.PushTransform(rotate);

  EmbeddedViewParams params;
  params.sizePoints = SkSize::Make(1, 1);;
  params.offsetPixels = SkPoint::Make(0, 0);
  params.mutatorsStack = stack;

  SkRect rect = params.GetBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), -sqrt(2)/2));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), sqrt(2)));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), sqrt(2)));
}

TEST(EmbeddedViewParams, GetBoundingRectWithTranslateScalingAndRotation) {
  MutatorsStack stack;
  SkMatrix translate = SkMatrix::MakeTrans(2, 2);
  SkMatrix scale = SkMatrix::MakeScale(3, 3);
  SkMatrix rotate;
  rotate.setRotate(90);
  stack.PushTransform(translate);
  stack.PushTransform(scale);
  stack.PushTransform(rotate);

  EmbeddedViewParams params;
  params.sizePoints = SkSize::Make(1, 1);;
  params.offsetPixels = SkPoint::Make(0, 0);
  params.mutatorsStack = stack;

  SkRect rect = params.GetBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), -1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 2));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 3));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 3));
}

}  // namespace testing
}  // namespace flutter
