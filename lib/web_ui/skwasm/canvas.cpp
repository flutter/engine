// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"
#include "wrappers.h"

using namespace Skwasm;

SKWASM_EXPORT void canvas_saveLayer(Canvas* canvas,
                                    Rect* rect,
                                    Paint* paint,
                                    ImageFilter* backdrop) {
  canvas->saveLayer(Canvas::SaveLayerRec(rect, paint, backdrop, 0));
}

SKWASM_EXPORT void canvas_save(Canvas* canvas) {
  canvas->save();
}

SKWASM_EXPORT void canvas_restore(Canvas* canvas) {
  canvas->restore();
}

SKWASM_EXPORT void canvas_restoreToCount(Canvas* canvas, int count) {
  canvas->restoreToCount(count);
}

SKWASM_EXPORT int canvas_getSaveCount(Canvas* canvas) {
  return canvas->getSaveCount();
}

SKWASM_EXPORT void canvas_translate(Canvas* canvas, Scalar dx, Scalar dy) {
  canvas->translate(dx, dy);
}

SKWASM_EXPORT void canvas_scale(Canvas* canvas, Scalar sx, Scalar sy) {
  canvas->scale(sx, sy);
}

SKWASM_EXPORT void canvas_rotate(Canvas* canvas, Scalar degrees) {
  canvas->rotate(degrees);
}

SKWASM_EXPORT void canvas_skew(Canvas* canvas, Scalar sx, Scalar sy) {
  canvas->skew(sx, sy);
}

SKWASM_EXPORT void canvas_transform(Canvas* canvas, const Matrix44* matrix44) {
  canvas->concat(*matrix44);
}

SKWASM_EXPORT void canvas_clipRect(Canvas* canvas,
                                   const Rect* rect,
                                   ClipOp op,
                                   bool antialias) {
  canvas->clipRect(*rect, op, antialias);
}

SKWASM_EXPORT void canvas_clipRRect(Canvas* canvas,
                                    const Scalar* rrectValues,
                                    bool antialias) {
  canvas->clipRRect(createRRect(rrectValues), antialias);
}

SKWASM_EXPORT void canvas_clipPath(Canvas* canvas, Path* path, bool antialias) {
  canvas->clipPath(*path, antialias);
}

SKWASM_EXPORT void canvas_drawColor(Canvas* canvas,
                                    Color color,
                                    BlendMode blendMode) {
  canvas->drawColor(color, blendMode);
}

SKWASM_EXPORT void canvas_drawLine(Canvas* canvas,
                                   Scalar x1,
                                   Scalar y1,
                                   Scalar x2,
                                   Scalar y2,
                                   Paint* paint) {
  canvas->drawLine(x1, y1, x2, y2, *paint);
}

SKWASM_EXPORT void canvas_drawPaint(Canvas* canvas, Paint* paint) {
  canvas->drawPaint(*paint);
}

SKWASM_EXPORT void canvas_drawRect(Canvas* canvas, Rect* rect, Paint* paint) {
  canvas->drawRect(*rect, *paint);
}

SKWASM_EXPORT void canvas_drawRRect(Canvas* canvas,
                                    const Scalar* rrectValues,
                                    Paint* paint) {
  canvas->drawRRect(createRRect(rrectValues), *paint);
}

SKWASM_EXPORT void canvas_drawDRRect(Canvas* canvas,
                                     const Scalar* outerRrectValues,
                                     const Scalar* innerRrectValues,
                                     Paint* paint) {
  canvas->drawDRRect(createRRect(outerRrectValues),
                     createRRect(innerRrectValues), *paint);
}

SKWASM_EXPORT void canvas_drawOval(Canvas* canvas,
                                   const Rect* rect,
                                   Paint* paint) {
  canvas->drawOval(*rect, *paint);
}

SKWASM_EXPORT void canvas_drawCircle(Canvas* canvas,
                                     Scalar x,
                                     Scalar y,
                                     Scalar radius,
                                     Paint* paint) {
  canvas->drawCircle(x, y, radius, *paint);
}

SKWASM_EXPORT void canvas_drawArc(Canvas* canvas,
                                  const Rect* rect,
                                  Scalar startAngleDegrees,
                                  Scalar sweepAngleDegrees,
                                  bool useCenter,
                                  Paint* paint) {
  canvas->drawArc(*rect, startAngleDegrees, sweepAngleDegrees, useCenter,
                  *paint);
}

SKWASM_EXPORT void canvas_drawPath(Canvas* canvas, Path* path, Paint* paint) {
  canvas->drawPath(*path, *paint);
}

SKWASM_EXPORT void canvas_drawShadow(Canvas* canvas,
                                     Path* path,
                                     Scalar elevation,
                                     Scalar devicePixelRatio,
                                     Color color,
                                     bool transparentOccluder) {
  drawShadowOnCanvas(canvas, path, elevation, devicePixelRatio, color,
                     transparentOccluder);
}

SKWASM_EXPORT void canvas_drawParagraph(Canvas* canvas,
                                        Paragraph* paragraph,
                                        Scalar x,
                                        Scalar y) {
  drawParagraphOnCanvas(canvas, paragraph, x, y);
}

SKWASM_EXPORT void canvas_drawPicture(Canvas* canvas, Picture* picture) {
  canvas->drawPicture(picture);
}

SKWASM_EXPORT void canvas_drawImage(Canvas* canvas,
                                    Image* image,
                                    Scalar offsetX,
                                    Scalar offsetY,
                                    Paint* paint,
                                    FilterQuality quality) {
  canvas->drawImage(image, offsetX, offsetY, samplingOptionsForQuality(quality),
                    paint);
}

SKWASM_EXPORT void canvas_drawImageRect(Canvas* canvas,
                                        Image* image,
                                        Rect* sourceRect,
                                        Rect* destRect,
                                        Paint* paint,
                                        FilterQuality quality) {
  canvas->drawImageRect(image, *sourceRect, *destRect,
                        samplingOptionsForQuality(quality), paint,
                        Canvas::kStrict_SrcRectConstraint);
}

SKWASM_EXPORT void canvas_drawImageNine(Canvas* canvas,
                                        Image* image,
                                        IRect* centerRect,
                                        Rect* destinationRect,
                                        Paint* paint,
                                        FilterQuality quality) {
  canvas->drawImageNine(image, *centerRect, *destinationRect,
                        filterModeForQuality(quality), paint);
}

SKWASM_EXPORT void canvas_drawVertices(Canvas* canvas,
                                       Vertices* vertices,
                                       BlendMode mode,
                                       Paint* paint) {
  canvas->drawVertices(vertices, mode, *paint);
}

SKWASM_EXPORT void canvas_drawPoints(Canvas* canvas,
                                     Canvas::PointMode mode,
                                     Point* points,
                                     int pointCount,
                                     Paint* paint) {
  canvas->drawPoints(mode, pointCount, points, *paint);
}

SKWASM_EXPORT void canvas_drawAtlas(Canvas* canvas,
                                    Image* atlas,
                                    RSXform* transforms,
                                    Rect* rects,
                                    Color* colors,
                                    int spriteCount,
                                    BlendMode mode,
                                    Rect* cullRect,
                                    Paint* paint) {
  canvas->drawAtlas(atlas, transforms, rects, colors, spriteCount, mode,
                    SamplingOptions{FilterMode::kLinear, MipmapMode::kNone},
                    cullRect, paint);
}

SKWASM_EXPORT void canvas_getTransform(Canvas* canvas, Matrix44* outTransform) {
  *outTransform = canvas->getLocalToDevice();
}

SKWASM_EXPORT void canvas_getLocalClipBounds(Canvas* canvas, Rect* outRect) {
  *outRect = canvas->getLocalClipBounds();
}

SKWASM_EXPORT void canvas_getDeviceClipBounds(Canvas* canvas, IRect* outRect) {
  *outRect = canvas->getDeviceClipBounds();
}
