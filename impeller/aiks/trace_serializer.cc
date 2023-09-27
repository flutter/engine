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
  FML_LOG(ERROR) << CanvasRecorderOpToString(op);
}

void TraceSerializer::Write(const Paint& paint) {}

void TraceSerializer::Write(const std::optional<Rect> optional_rect) {}

void TraceSerializer::Write(const std::shared_ptr<ImageFilter>& image_filter) {}

void TraceSerializer::Write(size_t size) {}

void TraceSerializer::Write(const Matrix& matrix) {}

void TraceSerializer::Write(const Vector3& vec3) {}

void TraceSerializer::Write(const Vector2& vec2) {}

void TraceSerializer::Write(const Radians& vec2) {}

void TraceSerializer::Write(const Path& path) {}

void TraceSerializer::Write(const std::vector<Point>& points) {}

void TraceSerializer::Write(const PointStyle& point_style) {}

void TraceSerializer::Write(const std::shared_ptr<Image>& image) {}

void TraceSerializer::Write(const SamplerDescriptor& sampler) {}

void TraceSerializer::Write(const Entity::ClipOperation& clip_op) {}

void TraceSerializer::Write(const Picture& clip_op) {}

void TraceSerializer::Write(const std::shared_ptr<TextFrame>& text_frame) {}

void TraceSerializer::Write(const std::shared_ptr<VerticesGeometry>& vertices) {
}

void TraceSerializer::Write(const BlendMode& blend_mode) {}

void TraceSerializer::Write(const std::vector<Matrix>& matrices) {}

void TraceSerializer::Write(const std::vector<Rect>& matrices) {}

void TraceSerializer::Write(const std::vector<Color>& matrices) {}
}  // namespace impeller
