// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/trace_serializer.h"
#include "flutter/fml/logging.h"

namespace impeller {

#define FLT_CANVAS_RECORDER_OP_TO_STRING(name) \
  case CanvasRecorderOp::name:                 \
    return #name

namespace {
std::string_view CanvasRecorderOpToString(CanvasRecorderOp op) {
  switch (op) {
    FLT_CANVAS_RECORDER_OP_TO_STRING(Save);
    FLT_CANVAS_RECORDER_OP_TO_STRING(SaveLayer);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Restore);
    FLT_CANVAS_RECORDER_OP_TO_STRING(RestoreToCount);
    FLT_CANVAS_RECORDER_OP_TO_STRING(ResetTransform);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Transform);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Concat);
    FLT_CANVAS_RECORDER_OP_TO_STRING(PreConcat);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Translate);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Scale2);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Scale3);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Skew);
    FLT_CANVAS_RECORDER_OP_TO_STRING(Rotate);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawPath);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawPaint);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawRect);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawRRect);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawCircle);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawPoints);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawImage);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawImageRect);
    FLT_CANVAS_RECORDER_OP_TO_STRING(ClipPath);
    FLT_CANVAS_RECORDER_OP_TO_STRING(ClipRect);
    FLT_CANVAS_RECORDER_OP_TO_STRING(ClipRRect);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawPicture);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawTextFrame);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawVertices);
    FLT_CANVAS_RECORDER_OP_TO_STRING(DrawAtlas);
  }
}
}  // namespace

TraceSerializer::TraceSerializer() {}

void TraceSerializer::Write(CanvasRecorderOp op) {
  FML_LOG(ERROR) << CanvasRecorderOpToString(op) << ":" << buffer_.str();
  buffer_.str("");
  buffer_.clear();
}

void TraceSerializer::Write(const Paint& paint) {
  buffer_ << "[Paint] ";
}

void TraceSerializer::Write(const std::optional<Rect> optional_rect) {
  buffer_ << "[std::optional<Rect>] ";
}

void TraceSerializer::Write(const std::shared_ptr<ImageFilter>& image_filter) {
  buffer_ << "[std::shared_ptr<ImageFilter>] ";
}

void TraceSerializer::Write(size_t size) {
  buffer_ << "[std::shared_ptr<ImageFilter>] ";
}

void TraceSerializer::Write(const Matrix& matrix) {
  buffer_ << "[Matrix] ";
}

void TraceSerializer::Write(const Vector3& vec3) {
  buffer_ << "[Vector3] ";
}

void TraceSerializer::Write(const Vector2& vec2) {
  buffer_ << "[Vector2] ";
}

void TraceSerializer::Write(const Radians& vec2) {
  buffer_ << "[Radians] ";
}

void TraceSerializer::Write(const Path& path) {
  buffer_ << "[Path] ";
}

void TraceSerializer::Write(const std::vector<Point>& points) {
  buffer_ << "[std::vector<Point>] ";
}

void TraceSerializer::Write(const PointStyle& point_style) {
  buffer_ << "[PointStyle] ";
}

void TraceSerializer::Write(const std::shared_ptr<Image>& image) {
  buffer_ << "[std::shared_ptr<Image>] ";
}

void TraceSerializer::Write(const SamplerDescriptor& sampler) {
  buffer_ << "[SamplerDescriptor] ";
}

void TraceSerializer::Write(const Entity::ClipOperation& clip_op) {
  buffer_ << "[Entity::ClipOperation] ";
}

void TraceSerializer::Write(const Picture& clip_op) {
  buffer_ << "[Picture] ";
}

void TraceSerializer::Write(const std::shared_ptr<TextFrame>& text_frame) {
  buffer_ << "[std::shared_ptr<TextFrame>] ";
}

void TraceSerializer::Write(const std::shared_ptr<VerticesGeometry>& vertices) {
  buffer_ << "[std::shared_ptr<VerticesGeometry>] ";
}

void TraceSerializer::Write(const BlendMode& blend_mode) {
  buffer_ << "[BlendMode] ";
}

void TraceSerializer::Write(const std::vector<Matrix>& matrices) {
  buffer_ << "[std::vector<Matrix>] ";
}

void TraceSerializer::Write(const std::vector<Rect>& matrices) {
  buffer_ << "[std::vector<Rect>] ";
}

void TraceSerializer::Write(const std::vector<Color>& matrices) {
  buffer_ << "[std::vector<Color>] ";
}
}  // namespace impeller
