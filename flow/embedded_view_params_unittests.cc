// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"
#include "flutter/fml/logging.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(EmbeddedViewParams, GetBoundingRectAfterMutationsWithNoMutations) {
  MutatorsStack stack;
  DlTransform matrix;

  EmbeddedViewParams params(matrix, DlFSize(1, 1), stack);
  const DlFRect& rect = params.finalBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 1));
}

TEST(EmbeddedViewParams, GetBoundingRectAfterMutationsWithScale) {
  MutatorsStack stack;
  DlTransform matrix = DlTransform::MakeScale(2, 2);
  stack.PushTransform(matrix);

  EmbeddedViewParams params(matrix, DlFSize(1, 1), stack);
  const DlFRect& rect = params.finalBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 2));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 2));
}

TEST(EmbeddedViewParams, GetBoundingRectAfterMutationsWithTranslate) {
  MutatorsStack stack;
  DlTransform matrix = DlTransform::MakeTranslate(1, 1);
  stack.PushTransform(matrix);

  EmbeddedViewParams params(matrix, DlFSize(1, 1), stack);
  const DlFRect& rect = params.finalBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 1));
}

TEST(EmbeddedViewParams, GetBoundingRectAfterMutationsWithRotation90) {
  MutatorsStack stack;
  DlTransform matrix;
  matrix.SetRotate(DlAngle::Degrees(90));
  stack.PushTransform(matrix);

  EmbeddedViewParams params(matrix, DlFSize(1, 1), stack);
  const DlFRect& rect = params.finalBoundingRect();

  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), -1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 1));
}

TEST(EmbeddedViewParams, GetBoundingRectAfterMutationsWithRotation45) {
  MutatorsStack stack;
  DlTransform matrix;
  matrix.SetRotate(DlAngle::Degrees(45));
  stack.PushTransform(matrix);

  EmbeddedViewParams params(matrix, DlFSize(1, 1), stack);
  const DlFRect& rect = params.finalBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), -sqrt(2) / 2));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 0));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), sqrt(2)));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), sqrt(2)));
}

TEST(EmbeddedViewParams,
     GetBoundingRectAfterMutationsWithTranslateScaleAndRotation) {
  DlTransform matrix = DlTransform::MakeTranslate(2, 2);
  matrix.ScaleInner(3, 3);
  matrix.RotateInner(DlAngle::Degrees(90));

  MutatorsStack stack;
  stack.PushTransform(matrix);

  EmbeddedViewParams params(matrix, DlFSize(1, 1), stack);
  const DlFRect& rect = params.finalBoundingRect();
  ASSERT_TRUE(SkScalarNearlyEqual(rect.x(), -1));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.y(), 2));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.width(), 3));
  ASSERT_TRUE(SkScalarNearlyEqual(rect.height(), 3));
}

}  // namespace testing
}  // namespace flutter
