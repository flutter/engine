// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/aiks/canvas.h"

namespace impeller {

enum CanvasRecorderOp {
  Save,
  SaveLayer,
  Restore,
  RestoreToCount,
  ResetTransform,
  Transform,
  Concat,
  PreConcat,
  Translate,
  Scale2,
  Scale3,
  Skew,
  Rotate,
  DrawPath,
  DrawPaint,
  DrawRect,
  DrawRRect,
  DrawCircle,
};

template <typename Serializer>
class CanvasRecorder {
 public:
  CanvasRecorder() : canvas_() {}

  explicit CanvasRecorder(Rect cull_rect) : canvas_(cull_rect) {}

  explicit CanvasRecorder(IRect cull_rect) : canvas_(cull_rect) {}

  ~CanvasRecorder() {}

  const Serializer& GetSerializer() const { return serializer_; }

  template <typename ReturnType>
  ReturnType ExecuteAndSerialize(CanvasRecorderOp op,
                                 ReturnType (Canvas::*canvasMethod)()) {
    serializer_.Write(op);
    return (canvas_.*canvasMethod)();
  }

  // template <typename FuncType, typename Arg0>
  // auto ExecuteAndSerialize(CanvasRecorderOp op,
  //                          FuncType canvasMethod,
  //                          Arg0&& arg0)
  //     -> decltype((std::declval<Canvas>().*
  //                  canvasMethod)(std::forward<Arg0>(arg0))) {
  //   serializer_.Write(op);
  //   serializer_.Write(arg0);
  //   return (canvas_.*canvasMethod)(std::forward<Arg0>(arg0));
  // }

  // template <typename FuncType, typename Arg0, typename Arg1>
  // auto ExecuteAndSerialize(CanvasRecorderOp op,
  //                          FuncType canvasMethod,
  //                          Arg0&& arg0,
  //                          Arg1&& arg1)
  //     -> decltype((std::declval<Canvas>().*
  //                  canvasMethod)(std::forward<Arg0>(arg0),
  //                                std::forward<Arg1>(arg1))) {
  //   serializer_.Write(op);
  //   serializer_.Write(arg0);
  //   serializer_.Write(arg1);
  //   return (canvas_.*canvasMethod)(std::forward<Arg0>(arg0),
  //                                  std::forward<Arg1>(arg1));
  // }

  // template <typename FuncType, typename Arg0, typename Arg1, typename Arg2>
  // auto ExecuteAndSerialize(CanvasRecorderOp op,
  //                          FuncType canvasMethod,
  //                          Arg0&& arg0,
  //                          Arg1&& arg1,
  //                          Arg2&& arg2)
  //     -> decltype((std::declval<Canvas>().*
  //                  canvasMethod)(std::forward<Arg0>(arg0),
  //                                std::forward<Arg1>(arg1),
  //                                std::forward<Arg2>(arg2))) {
  //   serializer_.Write(op);
  //   serializer_.Write(arg0);
  //   serializer_.Write(arg1);
  //   serializer_.Write(arg2);
  //   return (canvas_.*canvasMethod)(std::forward<Arg0>(arg0),
  //                                  std::forward<Arg1>(arg1),
  //                                  std::forward<Arg2>(arg2));
  // }

  template <typename FuncType, typename... Args>
  auto ExecuteAndSerialize(CanvasRecorderOp op,
                           FuncType canvasMethod,
                           Args&&... args)
      -> decltype((std::declval<Canvas>().*
                   canvasMethod)(std::forward<Args>(args)...)) {
    serializer_.Write(op);
    // Serialize each argument
    (serializer_.Write(std::forward<Args>(args)), ...);
    return (canvas_.*canvasMethod)(std::forward<Args>(args)...);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Canvas Static Polymorphism ////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  void Save() {
    return ExecuteAndSerialize(CanvasRecorderOp::Save, &Canvas::Save);
  }

  void SaveLayer(
      const Paint& paint,
      std::optional<Rect> bounds = std::nullopt,
      const std::shared_ptr<ImageFilter>& backdrop_filter = nullptr) {
    serializer_.Write(CanvasRecorderOp::SaveLayer);
    serializer_.Write(paint);
    serializer_.Write(bounds);
    serializer_.Write(backdrop_filter);
    return canvas_.SaveLayer(paint, bounds, backdrop_filter);
  }

  bool Restore() {
    return ExecuteAndSerialize(CanvasRecorderOp::Restore, &Canvas::Restore);
  }

  size_t GetSaveCount() const { return canvas_.GetSaveCount(); }

  void RestoreToCount(size_t count) {
    return ExecuteAndSerialize(CanvasRecorderOp::RestoreToCount,
                               &Canvas::RestoreToCount, count);
  }

  const Matrix& GetCurrentTransformation() const {
    return canvas_.GetCurrentTransformation();
  }

  const std::optional<Rect> GetCurrentLocalCullingBounds() const {
    return canvas_.GetCurrentLocalCullingBounds();
  }

  void ResetTransform() {
    return ExecuteAndSerialize(CanvasRecorderOp::ResetTransform,
                               &Canvas::ResetTransform);
  }

  void Transform(const Matrix& xformation) {
    return ExecuteAndSerialize(CanvasRecorderOp::Transform, &Canvas::Transform,
                               xformation);
  }

  void Concat(const Matrix& xformation) {
    return ExecuteAndSerialize(CanvasRecorderOp::Concat, &Canvas::Concat,
                               xformation);
  }

  void PreConcat(const Matrix& xformation) {
    return ExecuteAndSerialize(CanvasRecorderOp::PreConcat, &Canvas::PreConcat,
                               xformation);
  }

  void Translate(const Vector3& offset) {
    return ExecuteAndSerialize(CanvasRecorderOp::Translate, &Canvas::Translate,
                               offset);
  }

  void Scale(const Vector2& scale) {
    serializer_.Write(CanvasRecorderOp::Scale2);
    serializer_.Write(scale);
    return canvas_.Scale(scale);
  }

  void Scale(const Vector3& scale) {
    serializer_.Write(CanvasRecorderOp::Scale3);
    serializer_.Write(scale);
    return canvas_.Scale(scale);
  }

  void Skew(Scalar sx, Scalar sy) {
    return ExecuteAndSerialize(CanvasRecorderOp::Skew, &Canvas::Skew, sx, sy);
  }

  void Rotate(Radians radians) {
    return ExecuteAndSerialize(CanvasRecorderOp::Rotate, &Canvas::Rotate,
                               radians);
  }

  void DrawPath(const Path& path, const Paint& paint) {
    return ExecuteAndSerialize(CanvasRecorderOp::DrawPath, &Canvas::DrawPath,
                               path, paint);
  }

  void DrawPaint(const Paint& paint) {
    return ExecuteAndSerialize(CanvasRecorderOp::DrawPaint, &Canvas::DrawPaint,
                               paint);
  }

  void DrawRect(Rect rect, const Paint& paint) {
    return ExecuteAndSerialize(CanvasRecorderOp::DrawRect, &Canvas::DrawRect,
                               rect, paint);
  }

  void DrawRRect(Rect rect, Scalar corner_radius, const Paint& paint) {
    return ExecuteAndSerialize(CanvasRecorderOp::DrawRRect, &Canvas::DrawRRect,
                               rect, corner_radius, paint);
  }

  void DrawCircle(Point center, Scalar radius, const Paint& paint) {
    return ExecuteAndSerialize(CanvasRecorderOp::DrawCircle,
                               &Canvas::DrawCircle, center, radius, paint);
  }

  void DrawPoints(std::vector<Point>,
                  Scalar radius,
                  const Paint& paint,
                  PointStyle point_style) {}

  void DrawImage(const std::shared_ptr<Image>& image,
                 Point offset,
                 const Paint& paint,
                 SamplerDescriptor sampler = {}) {}

  void DrawImageRect(const std::shared_ptr<Image>& image,
                     Rect source,
                     Rect dest,
                     const Paint& paint,
                     SamplerDescriptor sampler = {}) {}

  void ClipPath(
      const Path& path,
      Entity::ClipOperation clip_op = Entity::ClipOperation::kIntersect) {}

  void ClipRect(
      const Rect& rect,
      Entity::ClipOperation clip_op = Entity::ClipOperation::kIntersect) {}

  void ClipRRect(
      const Rect& rect,
      Scalar corner_radius,
      Entity::ClipOperation clip_op = Entity::ClipOperation::kIntersect) {}

  void DrawPicture(const Picture& picture) {}

  void DrawTextFrame(const std::shared_ptr<TextFrame>& text_frame,
                     Point position,
                     const Paint& paint) {}

  void DrawVertices(const std::shared_ptr<VerticesGeometry>& vertices,
                    BlendMode blend_mode,
                    const Paint& paint) {}

  void DrawAtlas(const std::shared_ptr<Image>& atlas,
                 std::vector<Matrix> transforms,
                 std::vector<Rect> texture_coordinates,
                 std::vector<Color> colors,
                 BlendMode blend_mode,
                 SamplerDescriptor sampler,
                 std::optional<Rect> cull_rect,
                 const Paint& paint) {}

  Picture EndRecordingAsPicture() { return canvas_.EndRecordingAsPicture(); }

 private:
  Canvas canvas_;
  Serializer serializer_;
};

}  // namespace impeller
