// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"
#include "wrappers.h"

using namespace skia::textlayout;

using namespace Skwasm;

namespace {
// These numbers have been chosen empirically to give a result closest to the
// material spec.
// These values are also used by the CanvasKit renderer and the native engine.
// See:
//   flutter/display_list/skia/dl_sk_dispatcher.cc
//   flutter/lib/web_ui/lib/src/engine/canvaskit/util.dart
constexpr Scalar kShadowAmbientAlpha = 0.039;
constexpr Scalar kShadowSpotAlpha = 0.25;
constexpr Scalar kShadowLightRadius = 1.1;
constexpr Scalar kShadowLightHeight = 600.0;
constexpr Scalar kShadowLightXOffset = 0;
constexpr Scalar kShadowLightYOffset = -450;
}  // namespace

SKWASM_EXPORT void canvas_saveLayer(Canvas* canvas,
                                    SkRect* rect,
                                    SkPaint* paint,
                                    SkImageFilter* backdrop) {
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

SKWASM_EXPORT void canvas_translate(Canvas* canvas,
                                    Scalar dx,
                                    Scalar dy) {
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

SKWASM_EXPORT void canvas_transform(Canvas* canvas, const SkM44* matrix44) {
  canvas->concat(*matrix44);
}

SKWASM_EXPORT void canvas_clipRect(Canvas* canvas,
                                   const SkRect* rect,
                                   SkClipOp op,
                                   bool antialias) {
  canvas->clipRect(*rect, op, antialias);
}

SKWASM_EXPORT void canvas_clipRRect(Canvas* canvas,
                                    const Scalar* rrectValues,
                                    bool antialias) {
  canvas->clipRRect(createRRect(rrectValues), antialias);
}

SKWASM_EXPORT void canvas_clipPath(Canvas* canvas,
                                   SkPath* path,
                                   bool antialias) {
  canvas->clipPath(*path, antialias);
}

SKWASM_EXPORT void canvas_drawColor(Canvas* canvas,
                                    Color color,
                                    SkBlendMode blendMode) {
  canvas->drawColor(color, blendMode);
}

SKWASM_EXPORT void canvas_drawLine(Canvas* canvas,
                                   Scalar x1,
                                   Scalar y1,
                                   Scalar x2,
                                   Scalar y2,
                                   SkPaint* paint) {
  canvas->drawLine(x1, y1, x2, y2, *paint);
}

SKWASM_EXPORT void canvas_drawPaint(Canvas* canvas, SkPaint* paint) {
  canvas->drawPaint(*paint);
}

SKWASM_EXPORT void canvas_drawRect(Canvas* canvas,
                                   SkRect* rect,
                                   SkPaint* paint) {
  canvas->drawRect(*rect, *paint);
}

SKWASM_EXPORT void canvas_drawRRect(Canvas* canvas,
                                    const Scalar* rrectValues,
                                    SkPaint* paint) {
  canvas->drawRRect(createRRect(rrectValues), *paint);
}

SKWASM_EXPORT void canvas_drawDRRect(Canvas* canvas,
                                     const Scalar* outerRrectValues,
                                     const Scalar* innerRrectValues,
                                     SkPaint* paint) {
  canvas->drawDRRect(createRRect(outerRrectValues),
                     createRRect(innerRrectValues), *paint);
}

SKWASM_EXPORT void canvas_drawOval(Canvas* canvas,
                                   const SkRect* rect,
                                   SkPaint* paint) {
  canvas->drawOval(*rect, *paint);
}

SKWASM_EXPORT void canvas_drawCircle(Canvas* canvas,
                                     Scalar x,
                                     Scalar y,
                                     Scalar radius,
                                     SkPaint* paint) {
  canvas->drawCircle(x, y, radius, *paint);
}

SKWASM_EXPORT void canvas_drawArc(Canvas* canvas,
                                  const SkRect* rect,
                                  Scalar startAngleDegrees,
                                  Scalar sweepAngleDegrees,
                                  bool useCenter,
                                  SkPaint* paint) {
  canvas->drawArc(*rect, startAngleDegrees, sweepAngleDegrees, useCenter,
                  *paint);
}

SKWASM_EXPORT void canvas_drawPath(Canvas* canvas,
                                   SkPath* path,
                                   SkPaint* paint) {
  canvas->drawPath(*path, *paint);
}

SKWASM_EXPORT void canvas_drawShadow(Canvas* canvas,
                                     SkPath* path,
                                     Scalar elevation,
                                     Scalar devicePixelRatio,
                                     Color color,
                                     bool transparentOccluder) {
  Color inAmbient =
      SkColorSetA(color, kShadowAmbientAlpha * SkColorGetA(color));
  Color inSpot = SkColorSetA(color, kShadowSpotAlpha * SkColorGetA(color));
  Color outAmbient;
  Color outSpot;
  SkShadowUtils::ComputeTonalColors(inAmbient, inSpot, &outAmbient, &outSpot);
  uint32_t flags = transparentOccluder
                       ? SkShadowFlags::kTransparentOccluder_ShadowFlag
                       : SkShadowFlags::kNone_ShadowFlag;
  flags |= SkShadowFlags::kDirectionalLight_ShadowFlag;
  SkShadowUtils::DrawShadow(
      canvas, *path, Point3::Make(0.0f, 0.0f, elevation * devicePixelRatio),
      Point3::Make(kShadowLightXOffset, kShadowLightYOffset,
                     kShadowLightHeight * devicePixelRatio),
      devicePixelRatio * kShadowLightRadius, outAmbient, outSpot, flags);
}

SKWASM_EXPORT void canvas_drawParagraph(Canvas* canvas,
                                        Paragraph* paragraph,
                                        Scalar x,
                                        Scalar y) {
  paragraph->paint(canvas, x, y);
}

SKWASM_EXPORT void canvas_drawPicture(Canvas* canvas, SkPicture* picture) {
  canvas->drawPicture(picture);
}

SKWASM_EXPORT void canvas_drawImage(Canvas* canvas,
                                    SkImage* image,
                                    Scalar offsetX,
                                    Scalar offsetY,
                                    SkPaint* paint,
                                    FilterQuality quality) {
  canvas->drawImage(image, offsetX, offsetY, samplingOptionsForQuality(quality),
                    paint);
}

SKWASM_EXPORT void canvas_drawImageRect(Canvas* canvas,
                                        SkImage* image,
                                        SkRect* sourceRect,
                                        SkRect* destRect,
                                        SkPaint* paint,
                                        FilterQuality quality) {
  canvas->drawImageRect(image, *sourceRect, *destRect,
                        samplingOptionsForQuality(quality), paint,
                        Canvas::kStrict_SrcRectConstraint);
}

SKWASM_EXPORT void canvas_drawImageNine(Canvas* canvas,
                                        SkImage* image,
                                        SkIRect* centerRect,
                                        SkRect* destinationRect,
                                        SkPaint* paint,
                                        FilterQuality quality) {
  canvas->drawImageNine(image, *centerRect, *destinationRect,
                        filterModeForQuality(quality), paint);
}

SKWASM_EXPORT void canvas_drawVertices(Canvas* canvas,
                                       SkVertices* vertices,
                                       SkBlendMode mode,
                                       SkPaint* paint) {
  canvas->drawVertices(sk_ref_sp<SkVertices>(vertices), mode, *paint);
}

SKWASM_EXPORT void canvas_drawPoints(Canvas* canvas,
                                     Canvas::PointMode mode,
                                     Point* points,
                                     int pointCount,
                                     SkPaint* paint) {
  canvas->drawPoints(mode, pointCount, points, *paint);
}

SKWASM_EXPORT void canvas_drawAtlas(Canvas* canvas,
                                    SkImage* atlas,
                                    SkRSXform* transforms,
                                    SkRect* rects,
                                    Color* colors,
                                    int spriteCount,
                                    SkBlendMode mode,
                                    SkRect* cullRect,
                                    SkPaint* paint) {
  canvas->drawAtlas(
      atlas, transforms, rects, colors, spriteCount, mode,
      SkSamplingOptions{SkFilterMode::kLinear, SkMipmapMode::kNone}, cullRect,
      paint);
}

SKWASM_EXPORT void canvas_getTransform(Canvas* canvas, SkM44* outTransform) {
  *outTransform = canvas->getLocalToDevice();
}

SKWASM_EXPORT void canvas_getLocalClipBounds(Canvas* canvas,
                                             SkRect* outRect) {
  *outRect = canvas->getLocalClipBounds();
}

SKWASM_EXPORT void canvas_getDeviceClipBounds(Canvas* canvas,
                                              SkIRect* outRect) {
  *outRect = canvas->getDeviceClipBounds();
}
