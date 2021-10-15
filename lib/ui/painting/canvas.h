// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_CANVAS_H_
#define FLUTTER_LIB_UI_PAINTING_CANVAS_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/paint.h"
#include "flutter/lib/ui/painting/path.h"
#include "flutter/lib/ui/painting/picture.h"
#include "flutter/lib/ui/painting/picture_recorder.h"
#include "flutter/lib/ui/painting/rrect.h"
#include "flutter/lib/ui/painting/vertices.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace flutter {
class CanvasImage;

class Canvas : public RefCountedDartWrappable<Canvas> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(Canvas);

  PaintData place_holder;

 public:
  static fml::RefPtr<Canvas> Create(PictureRecorder* recorder,
                                    double left,
                                    double top,
                                    double right,
                                    double bottom);

  static fml::RefPtr<Canvas> CreateOrThrow(Dart_Handle wrapper,
                                           PictureRecorder* recorder,
                                           double left,
                                           double top,
                                           double right,
                                           double bottom);

  ~Canvas() override;

  void save();
  void saveLayerWithoutBounds(const Paint& paint, const PaintData& paint_data);
  void saveLayerWithoutBoundsHandle(Dart_Handle paint_objects,
                                    Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    saveLayerWithoutBounds(paint, place_holder);
  }

  void saveLayer(double left,
                 double top,
                 double right,
                 double bottom,
                 const Paint& paint,
                 const PaintData& paint_data);
  void saveLayerHandle(double left,
                       double top,
                       double right,
                       double bottom,
                       Dart_Handle paint_objects,
                       Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    saveLayer(left, top, right, bottom, paint, place_holder);
  }

  void restore();
  int getSaveCount();

  void translate(double dx, double dy);
  void scale(double sx, double sy);
  void rotate(double radians);
  void skew(double sx, double sy);
  void transform(const tonic::Float64List& matrix4);

  void clipRect(double left,
                double top,
                double right,
                double bottom,
                SkClipOp clipOp,
                bool doAntiAlias = true);
  void clipRRect(const RRect& rrect, bool doAntiAlias = true);
  void clipPath(const CanvasPath* path, bool doAntiAlias = true);

  void drawColor(SkColor color, SkBlendMode blend_mode);

  void drawLine(double x1,
                double y1,
                double x2,
                double y2,
                const Paint& paint,
                const PaintData& paint_data);
  void drawLineHandle(double x1,
                      double y1,
                      double x2,
                      double y2,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawLine(x1, y1, x2, y2, paint, place_holder);
  }

  void drawPaint(const Paint& paint, const PaintData& paint_data);
  void drawPaintHandle(Dart_Handle paint_objects, Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawPaint(paint, place_holder);
  }

  void drawRect(double left,
                double top,
                double right,
                double bottom,
                const Paint& paint,
                const PaintData& paint_data);
  void drawRectHandle(double left,
                      double top,
                      double right,
                      double bottom,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawRect(left, top, right, bottom, paint, place_holder);
  }

  void drawRRect(const RRect& rrect,
                 const Paint& paint,
                 const PaintData& paint_data);
  void drawRRectHandle(const RRect& rrect,
                       Dart_Handle paint_objects,
                       Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawRRect(rrect, paint, place_holder);
  }

  void drawDRRect(const RRect& outer,
                  const RRect& inner,
                  const Paint& paint,
                  const PaintData& paint_data);
  void drawDRRectHandle(const RRect& outer,
                        const RRect& inner,
                        Dart_Handle paint_objects,
                        Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawDRRect(outer, inner, paint, place_holder);
  }

  void drawOval(double left,
                double top,
                double right,
                double bottom,
                const Paint& paint,
                const PaintData& paint_data);
  void drawOvalHandle(double left,
                      double top,
                      double right,
                      double bottom,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawOval(left, top, right, bottom, paint, place_holder);
  }

  void drawCircle(double x,
                  double y,
                  double radius,
                  const Paint& paint,
                  const PaintData& paint_data);
  void drawCircleHandle(double x,
                        double y,
                        double radius,
                        Dart_Handle paint_objects,
                        Dart_Handle paint_data) {
    if (!canvas_) {
      return;
    }
    Paint paint(paint_objects, paint_data);
    canvas_->drawCircle(x, y, radius, *paint.paint());
  }

  void drawArc(double left,
               double top,
               double right,
               double bottom,
               double startAngle,
               double sweepAngle,
               bool useCenter,
               const Paint& paint,
               const PaintData& paint_data);
  void drawArcHandle(double left,
                     double top,
                     double right,
                     double bottom,
                     double startAngle,
                     double sweepAngle,
                     bool useCenter,
                     Dart_Handle paint_objects,
                     Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawArc(left, top, right, bottom, startAngle, sweepAngle, useCenter, paint,
            place_holder);
  }

  void drawPath(const CanvasPath* path,
                const Paint& paint,
                const PaintData& paint_data);
  void drawPathHandle(const CanvasPath* path,
                      Dart_Handle paint_objects,
                      Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawPath(path, paint, place_holder);
  }

  void drawImage(const CanvasImage* image,
                 double x,
                 double y,
                 const Paint& paint,
                 const PaintData& paint_data,
                 int filterQualityIndex);
  void drawImageHandle(const CanvasImage* image,
                       double x,
                       double y,
                       Dart_Handle paint_objects,
                       Dart_Handle paint_data,
                       int filterQualityIndex) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawImage(image, x, y, paint, place_holder, filterQualityIndex);
  }

  void drawImageRect(const CanvasImage* image,
                     double src_left,
                     double src_top,
                     double src_right,
                     double src_bottom,
                     double dst_left,
                     double dst_top,
                     double dst_right,
                     double dst_bottom,
                     const Paint& paint,
                     const PaintData& paint_data,
                     int filterQualityIndex);
  void drawImageRectHandle(const CanvasImage* image,
                           double src_left,
                           double src_top,
                           double src_right,
                           double src_bottom,
                           double dst_left,
                           double dst_top,
                           double dst_right,
                           double dst_bottom,
                           Dart_Handle paint_objects,
                           Dart_Handle paint_data,
                           int filterQualityIndex) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawImageRect(image, src_left, src_top, src_right, src_bottom, dst_left,
                  dst_top, dst_right, dst_bottom, paint, place_holder,
                  filterQualityIndex);
  }

  void drawImageNine(const CanvasImage* image,
                     double center_left,
                     double center_top,
                     double center_right,
                     double center_bottom,
                     double dst_left,
                     double dst_top,
                     double dst_right,
                     double dst_bottom,
                     const Paint& paint,
                     const PaintData& paint_data,
                     int bitmapSamplingIndex);
  void drawImageNineHandle(const CanvasImage* image,
                           double center_left,
                           double center_top,
                           double center_right,
                           double center_bottom,
                           double dst_left,
                           double dst_top,
                           double dst_right,
                           double dst_bottom,
                           Dart_Handle paint_objects,
                           Dart_Handle paint_data,
                           int bitmapSamplingIndex) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawImageNine(image, center_left, center_top, center_right, center_bottom,
                  dst_left, dst_top, dst_right, dst_bottom, paint, place_holder,
                  bitmapSamplingIndex);
  }

  void drawPicture(Picture* picture);

  // The paint argument is first for the following functions because Paint
  // unwraps a number of C++ objects. Once we create a view unto a
  // Float32List, we cannot re-enter the VM to unwrap objects. That means we
  // either need to process the paint argument first.

  void drawPoints(const Paint& paint,
                  const PaintData& paint_data,
                  SkCanvas::PointMode point_mode,
                  const tonic::Float32List& points);
  void drawPointsHandle(Dart_Handle paint_objects,
                        Dart_Handle paint_data,
                        SkCanvas::PointMode point_mode,
                        const tonic::Float32List& points) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawPoints(paint, place_holder, point_mode, points);
  }

  void drawVertices(const Vertices* vertices,
                    SkBlendMode blend_mode,
                    const Paint& paint,
                    const PaintData& paint_data);
  void drawVerticesHandle(const Vertices* vertices,
                          SkBlendMode blend_mode,
                          Dart_Handle paint_objects,
                          Dart_Handle paint_data) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawVertices(vertices, blend_mode, paint, place_holder);
  }

  void drawAtlas(const Paint& paint,
                 const PaintData& paint_data,
                 int filterQualityIndex,
                 CanvasImage* atlas,
                 const tonic::Float32List& transforms,
                 const tonic::Float32List& rects,
                 const tonic::Int32List& colors,
                 SkBlendMode blend_mode,
                 const tonic::Float32List& cull_rect);
  void drawAtlasHandle(Dart_Handle paint_objects,
                       Dart_Handle paint_data,
                       int filterQualityIndex,
                       CanvasImage* atlas,
                       const tonic::Float32List& transforms,
                       const tonic::Float32List& rects,
                       const tonic::Int32List& colors,
                       SkBlendMode blend_mode,
                       const tonic::Float32List& cull_rect) {
    Paint paint(paint_objects, paint_data);
    PaintData place_holder;
    drawAtlas(paint, place_holder, filterQualityIndex, atlas, transforms, rects,
              colors, blend_mode, cull_rect);
  }

  void drawShadow(const CanvasPath* path,
                  SkColor color,
                  double elevation,
                  bool transparentOccluder);

  SkCanvas* canvas() const { return canvas_; }
  void Invalidate();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  explicit Canvas(SkCanvas* canvas);

  // The SkCanvas is supplied by a call to SkPictureRecorder::beginRecording,
  // which does not transfer ownership.  For this reason, we hold a raw
  // pointer and manually set to null in Clear.
  SkCanvas* canvas_;

  // A copy of the recorder used by the SkCanvas->DisplayList adapter for cases
  // where we cannot record the SkCanvas method call through the various OnOp()
  // virtual methods or where we can be more efficient by talking directly in
  // the DisplayList operation lexicon. The recorder has a method for recording
  // paint attributes from an SkPaint and an operation type as well as access
  // to the raw DisplayListBuilder for emitting custom rendering operations.
  sk_sp<DisplayListCanvasRecorder> display_list_recorder_;
  sk_sp<DisplayListBuilder> builder() {
    return display_list_recorder_->builder();
  }
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_CANVAS_H_
