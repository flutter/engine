// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"
#include "gtest/gtest.h"

TEST(MutatorsStack, Initialization) {
  flutter::MutatorsStack stack;
  ASSERT_TRUE(true);
}

TEST(MutatorsStack, PushClipRect) {
  flutter::MutatorsStack stack;
  SkRect rect;
  stack.pushClipRect(rect);
  auto iter = stack.bottom();
  ASSERT_TRUE(iter->type() == flutter::MutatorType::clip_rect);
  ASSERT_TRUE(iter->rect() == rect);
}

TEST(MutatorsStack, PushClipRRect) {
  flutter::MutatorsStack stack;
  SkRRect rrect;
  stack.pushClipRRect(rrect);
  auto iter = stack.bottom();
  ASSERT_TRUE(iter->type() == flutter::MutatorType::clip_rrect);
  ASSERT_TRUE(iter->rrect() == rrect);
}

TEST(MutatorsStack, PushTransform) {
  flutter::MutatorsStack stack;
  SkMatrix matrix;
  stack.pushTransform(matrix);
  auto iter = stack.bottom();
  ASSERT_TRUE(iter->type() == flutter::MutatorType::transform);
  ASSERT_TRUE(iter->matrix() == matrix);
}

TEST(MutatorsStack, Pop) {
  flutter::MutatorsStack stack;
  SkMatrix matrix;
  stack.pushTransform(matrix);
  stack.pop();
  auto iter = stack.bottom();
  ASSERT_TRUE(iter == stack.top());
}

TEST(MutatorsStack, Traversal) {
  flutter::MutatorsStack stack;
  SkMatrix matrix;
  stack.pushTransform(matrix);
  SkRect rect;
  stack.pushClipRect(rect);
  SkRRect rrect;
  stack.pushClipRRect(rrect);
  auto iter = stack.bottom();
  int index = 0;
  while (iter != stack.top()) {
    switch (index) {
      case 0:
        ASSERT_TRUE(iter->type() == flutter::MutatorType::clip_rrect);
        ASSERT_TRUE(iter->rrect() == rrect);
        break;
      case 1:
        ASSERT_TRUE(iter->type() == flutter::MutatorType::clip_rect);
        ASSERT_TRUE(iter->rect() == rect);
        break;
      case 2:
        ASSERT_TRUE(iter->type() == flutter::MutatorType::transform);
        ASSERT_TRUE(iter->matrix() == matrix);
        break;
      default:
        break;
    }
    ++iter;
    ++index;
  }
}

TEST(MutatorsStack, Equality) {
  flutter::MutatorsStack stack;
  SkMatrix matrix = SkMatrix::MakeScale(1, 1);
  stack.pushTransform(matrix);
  SkRect rect = SkRect::MakeEmpty();
  stack.pushClipRect(rect);
  SkRRect rrect = SkRRect::MakeEmpty();
  stack.pushClipRRect(rrect);

  flutter::MutatorsStack stackOther;
  SkMatrix matrixOther = SkMatrix::MakeScale(1, 1);
  stackOther.pushTransform(matrixOther);
  SkRect rectOther = SkRect::MakeEmpty();
  stackOther.pushClipRect(rectOther);
  SkRRect rrectOther = SkRRect::MakeEmpty();
  stackOther.pushClipRRect(rrectOther);

  ASSERT_TRUE(stack == stackOther);
}

TEST(Mutator, Equality) {
  SkMatrix matrix;
  flutter::Mutator mutator = flutter::Mutator(matrix);
  flutter::Mutator otherMutator = flutter::Mutator(matrix);
  ASSERT_TRUE(mutator == otherMutator);

  SkRect rect;
  flutter::Mutator mutator2 = flutter::Mutator(rect);
  flutter::Mutator otherMutator2 = flutter::Mutator(rect);
  ASSERT_TRUE(mutator2 == otherMutator2);

  SkRRect rrect;
  flutter::Mutator mutator3 = flutter::Mutator(rrect);
  flutter::Mutator otherMutator3 =flutter::Mutator(rrect);
  ASSERT_TRUE(mutator3 == otherMutator3);

  ASSERT_FALSE(mutator2 == mutator);
}

TEST(Mutator, UnEquality) {
  SkRect rect;
  flutter::Mutator mutator = flutter::Mutator(rect);
  SkMatrix matrix;
  flutter::Mutator notEqualMutator = flutter::Mutator(matrix);
  ASSERT_TRUE(notEqualMutator != mutator);
}
