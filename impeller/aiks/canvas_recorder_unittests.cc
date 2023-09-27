// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/aiks/canvas_recorder.h"
namespace impeller {
namespace testing {

namespace {
class Serializer {
 public:
  void Write(CanvasRecorderOp op) { last_op_ = op; }

  void Write(const Paint& paint) {}

  void Write(const std::optional<Rect> optional_rect) {}

  void Write(const std::shared_ptr<ImageFilter>& image_filter) {}

  void Write(size_t size) {}

  void Write(const Matrix& matrix) {}

  void Write(const Vector3& vec3) {}

  void Write(const Vector2& vec2) {}

  void Write(const Radians& vec2) {}

  void Write(const Path& path) {}

  CanvasRecorderOp last_op_;
};
}  // namespace

TEST(CanvasRecorder, Save) {
  CanvasRecorder<Serializer> recorder;
  recorder.Save();
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Save);
}

TEST(CanvasRecorder, SaveLayer) {
  CanvasRecorder<Serializer> recorder;
  Paint paint;
  recorder.SaveLayer(paint);
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::SaveLayer);
}

TEST(CanvasRecorder, Restore) {
  CanvasRecorder<Serializer> recorder;
  recorder.Restore();
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Restore);
}

TEST(CanvasRecorder, RestoreToCount) {
  CanvasRecorder<Serializer> recorder;
  recorder.Save();
  recorder.RestoreToCount(0);
  ASSERT_EQ(recorder.GetSerializer().last_op_,
            CanvasRecorderOp::RestoreToCount);
}

TEST(CanvasRecorder, ResetTransform) {
  CanvasRecorder<Serializer> recorder;
  recorder.ResetTransform();
  ASSERT_EQ(recorder.GetSerializer().last_op_,
            CanvasRecorderOp::ResetTransform);
}

TEST(CanvasRecorder, Transform) {
  CanvasRecorder<Serializer> recorder;
  recorder.Transform(Matrix());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Transform);
}

TEST(CanvasRecorder, Concat) {
  CanvasRecorder<Serializer> recorder;
  recorder.Concat(Matrix());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Concat);
}

TEST(CanvasRecorder, PreConcat) {
  CanvasRecorder<Serializer> recorder;
  recorder.PreConcat(Matrix());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::PreConcat);
}

TEST(CanvasRecorder, Translate) {
  CanvasRecorder<Serializer> recorder;
  recorder.Translate(Vector3());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Translate);
}

TEST(CanvasRecorder, Scale2) {
  CanvasRecorder<Serializer> recorder;
  recorder.Scale(Vector2());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Scale2);
}

TEST(CanvasRecorder, Scale3) {
  CanvasRecorder<Serializer> recorder;
  recorder.Scale(Vector3());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Scale3);
}

TEST(CanvasRecorder, Skew) {
  CanvasRecorder<Serializer> recorder;
  recorder.Skew(0, 0);
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Skew);
}

TEST(CanvasRecorder, Rotate) {
  CanvasRecorder<Serializer> recorder;
  recorder.Rotate(Radians(0));
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::Rotate);
}

TEST(CanvasRecorder, DrawPath) {
  CanvasRecorder<Serializer> recorder;
  recorder.DrawPath(Path(), Paint());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::DrawPath);
}

TEST(CanvasRecorder, DrawPaint) {
  CanvasRecorder<Serializer> recorder;
  recorder.DrawPaint(Paint());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::DrawPaint);
}

TEST(CanvasRecorder, DrawRect) {
  CanvasRecorder<Serializer> recorder;
  recorder.DrawRect(Rect(), Paint());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::DrawRect);
}

TEST(CanvasRecorder, DrawRRect) {
  CanvasRecorder<Serializer> recorder;
  recorder.DrawRRect(Rect(), 0, Paint());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::DrawRRect);
}

TEST(CanvasRecorder, DrawCircle) {
  CanvasRecorder<Serializer> recorder;
  recorder.DrawCircle(Point(), 0, Paint());
  ASSERT_EQ(recorder.GetSerializer().last_op_, CanvasRecorderOp::DrawCircle);
}

}  // namespace testing
}  // namespace impeller
