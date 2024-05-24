// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"

using namespace Skwasm;

SKWASM_EXPORT SkPath* path_create() {
  return new SkPath();
}

SKWASM_EXPORT void path_dispose(SkPath* path) {
  delete path;
}

SKWASM_EXPORT SkPath* path_copy(SkPath* path) {
  return new SkPath(*path);
}

SKWASM_EXPORT void path_setFillType(SkPath* path, SkPathFillType fillType) {
  path->setFillType(fillType);
}

SKWASM_EXPORT SkPathFillType path_getFillType(SkPath* path) {
  return path->getFillType();
}

SKWASM_EXPORT void path_moveTo(SkPath* path, Scalar x, Scalar y) {
  path->moveTo(x, y);
}

SKWASM_EXPORT void path_relativeMoveTo(SkPath* path, Scalar x, Scalar y) {
  path->rMoveTo(x, y);
}

SKWASM_EXPORT void path_lineTo(SkPath* path, Scalar x, Scalar y) {
  path->lineTo(x, y);
}

SKWASM_EXPORT void path_relativeLineTo(SkPath* path, Scalar x, Scalar y) {
  path->rLineTo(x, y);
}

SKWASM_EXPORT void path_quadraticBezierTo(SkPath* path,
                                          Scalar x1,
                                          Scalar y1,
                                          Scalar x2,
                                          Scalar y2) {
  path->quadTo(x1, y1, x2, y2);
}

SKWASM_EXPORT void path_relativeQuadraticBezierTo(SkPath* path,
                                                  Scalar x1,
                                                  Scalar y1,
                                                  Scalar x2,
                                                  Scalar y2) {
  path->rQuadTo(x1, y1, x2, y2);
}

SKWASM_EXPORT void path_cubicTo(SkPath* path,
                                Scalar x1,
                                Scalar y1,
                                Scalar x2,
                                Scalar y2,
                                Scalar x3,
                                Scalar y3) {
  path->cubicTo(x1, y1, x2, y2, x3, y3);
}

SKWASM_EXPORT void path_relativeCubicTo(SkPath* path,
                                        Scalar x1,
                                        Scalar y1,
                                        Scalar x2,
                                        Scalar y2,
                                        Scalar x3,
                                        Scalar y3) {
  path->rCubicTo(x1, y1, x2, y2, x3, y3);
}

SKWASM_EXPORT void path_conicTo(SkPath* path,
                                Scalar x1,
                                Scalar y1,
                                Scalar x2,
                                Scalar y2,
                                Scalar w) {
  path->conicTo(x1, y1, x2, y2, w);
}

SKWASM_EXPORT void path_relativeConicTo(SkPath* path,
                                        Scalar x1,
                                        Scalar y1,
                                        Scalar x2,
                                        Scalar y2,
                                        Scalar w) {
  path->rConicTo(x1, y1, x2, y2, w);
}

SKWASM_EXPORT void path_arcToOval(SkPath* path,
                                  const SkRect* rect,
                                  Scalar startAngle,
                                  Scalar sweepAngle,
                                  bool forceMoveTo) {
  path->arcTo(*rect, startAngle, sweepAngle, forceMoveTo);
}

SKWASM_EXPORT void path_arcToRotated(SkPath* path,
                                     Scalar rx,
                                     Scalar ry,
                                     Scalar xAxisRotate,
                                     SkPath::ArcSize arcSize,
                                     SkPathDirection pathDirection,
                                     Scalar x,
                                     Scalar y) {
  path->arcTo(rx, ry, xAxisRotate, arcSize, pathDirection, x, y);
}

SKWASM_EXPORT void path_relativeArcToRotated(SkPath* path,
                                             Scalar rx,
                                             Scalar ry,
                                             Scalar xAxisRotate,
                                             SkPath::ArcSize arcSize,
                                             SkPathDirection pathDirection,
                                             Scalar x,
                                             Scalar y) {
  path->rArcTo(rx, ry, xAxisRotate, arcSize, pathDirection, x, y);
}

SKWASM_EXPORT void path_addRect(SkPath* path, const SkRect* rect) {
  path->addRect(*rect);
}

SKWASM_EXPORT void path_addOval(SkPath* path, const SkRect* oval) {
  path->addOval(*oval, SkPathDirection::kCW, 1);
}

SKWASM_EXPORT void path_addArc(SkPath* path,
                               const SkRect* oval,
                               Scalar startAngle,
                               Scalar sweepAngle) {
  path->addArc(*oval, startAngle, sweepAngle);
}

SKWASM_EXPORT void path_addPolygon(SkPath* path,
                                   const Point* points,
                                   int count,
                                   bool close) {
  path->addPoly(points, count, close);
}

SKWASM_EXPORT void path_addRRect(SkPath* path, const Scalar* rrectValues) {
  path->addRRect(createRRect(rrectValues), SkPathDirection::kCW);
}

SKWASM_EXPORT void path_addPath(SkPath* path,
                                const SkPath* other,
                                const Scalar* matrix33,
                                SkPath::AddPathMode extendPath) {
  path->addPath(*other, createMatrix(matrix33), extendPath);
}

SKWASM_EXPORT void path_close(SkPath* path) {
  path->close();
}

SKWASM_EXPORT void path_reset(SkPath* path) {
  path->reset();
}

SKWASM_EXPORT bool path_contains(SkPath* path, Scalar x, Scalar y) {
  return path->contains(x, y);
}

SKWASM_EXPORT void path_transform(SkPath* path, const Scalar* matrix33) {
  path->transform(createMatrix(matrix33));
}

SKWASM_EXPORT void path_getBounds(SkPath* path, SkRect* rect) {
  *rect = path->getBounds();
}

SKWASM_EXPORT SkPath* path_combine(SkPathOp operation,
                                   const SkPath* path1,
                                   const SkPath* path2) {
  SkPath* output = new SkPath();
  if (Op(*path1, *path2, operation, output)) {
    output->setFillType(path1->getFillType());
    return output;
  } else {
    delete output;
    return nullptr;
  }
}
