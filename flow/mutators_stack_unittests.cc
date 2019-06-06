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
  ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::clip_rect);
  ASSERT_TRUE(iter->get()->rect() == rect);
}

TEST(MutatorsStack, PushClipRRect) {
  flutter::MutatorsStack stack;
  SkRRect rrect;
  stack.pushClipRRect(rrect);
  auto iter = stack.bottom();
  ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::clip_rrect);
  ASSERT_TRUE(iter->get()->rrect() == rrect);
}

TEST(MutatorsStack, PushClipPath) {
  flutter::MutatorsStack stack;
  SkPath path;
  stack.pushClipPath(path);
  auto iter = stack.bottom();
  ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::clip_path);
  ASSERT_TRUE(iter->get()->path() == path);
}

TEST(MutatorsStack, PushTransform) {
  flutter::MutatorsStack stack;
  SkMatrix matrix;
  stack.pushTransform(matrix);
  auto iter = stack.bottom();
  ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::transform);
  ASSERT_TRUE(iter->get()->matrix() == matrix);
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
        ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::clip_rrect);
        ASSERT_TRUE(iter->get()->rrect() == rrect);
        break;
      case 1:
        ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::clip_rect);
        ASSERT_TRUE(iter->get()->rect() == rect);
        break;
      case 2:
        ASSERT_TRUE(iter->get()->type() == flutter::MutatorType::transform);
        ASSERT_TRUE(iter->get()->matrix() == matrix);
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
  SkPath path;
  stack.pushClipPath(path);

  flutter::MutatorsStack stackOther;
  SkMatrix matrixOther = SkMatrix::MakeScale(1, 1);
  stackOther.pushTransform(matrixOther);
  SkRect rectOther = SkRect::MakeEmpty();
  stackOther.pushClipRect(rectOther);
  SkRRect rrectOther = SkRRect::MakeEmpty();
  stackOther.pushClipRRect(rrectOther);
  SkPath otherPath;
  stackOther.pushClipPath(otherPath);

  ASSERT_TRUE(stack == stackOther);
}

TEST(Mutator, Equality) {
  flutter::Mutator mutator;
  flutter::Mutator otherMutator;
  mutator.setType(flutter::MutatorType::transform);
  otherMutator.setType(flutter::MutatorType::transform);
  SkMatrix matrix;
  mutator.setMatrix(matrix);
  otherMutator.setMatrix(matrix);
  ASSERT_TRUE(mutator == otherMutator);

  mutator.setType(flutter::MutatorType::clip_rect);
  otherMutator.setType(flutter::MutatorType::clip_rect);
  SkRect rect;
  mutator.setRect(rect);
  otherMutator.setRect(rect);
  ASSERT_TRUE(mutator == otherMutator);

  mutator.setType(flutter::MutatorType::clip_rrect);
  otherMutator.setType(flutter::MutatorType::clip_rrect);
  SkRRect rrect;
  mutator.setRRect(rrect);
  otherMutator.setRRect(rrect);
  ASSERT_TRUE(mutator == otherMutator);

  mutator.setType(flutter::MutatorType::clip_path);
  otherMutator.setType(flutter::MutatorType::clip_path);
  SkPath path;
  mutator.setPath(path);
  otherMutator.setPath(path);
  ASSERT_TRUE(mutator == otherMutator);

  flutter::Mutator notEqualMutator;
  notEqualMutator.setType(flutter::MutatorType::transform);
  notEqualMutator.setMatrix(matrix);
  ASSERT_FALSE(notEqualMutator == mutator);
}

TEST(Mutator, UnEquality) {
  flutter::Mutator mutator;
  mutator.setType(flutter::MutatorType::clip_rect);
  SkRect rect;
  mutator.setRect(rect);

  SkMatrix matrix;
  flutter::Mutator notEqualMutator;
  notEqualMutator.setType(flutter::MutatorType::transform);
  notEqualMutator.setMatrix(matrix);
  ASSERT_TRUE(notEqualMutator != mutator);
}
