// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"

using namespace Skwasm;

SKWASM_EXPORT Path* path_create() {
  return new Path();
}

SKWASM_EXPORT void path_dispose(Path* path) {
  delete path;
}

SKWASM_EXPORT Path* path_copy(Path* path) {
  return new Path(*path);
}

SKWASM_EXPORT void path_setFillType(Path* path, PathFillType fillType) {
  path->setFillType(fillType);
}

SKWASM_EXPORT PathFillType path_getFillType(Path* path) {
  return path->getFillType();
}

SKWASM_EXPORT void path_moveTo(Path* path, Scalar x, Scalar y) {
  path->moveTo(x, y);
}

SKWASM_EXPORT void path_relativeMoveTo(Path* path, Scalar x, Scalar y) {
  path->rMoveTo(x, y);
}

SKWASM_EXPORT void path_lineTo(Path* path, Scalar x, Scalar y) {
  path->lineTo(x, y);
}

SKWASM_EXPORT void path_relativeLineTo(Path* path, Scalar x, Scalar y) {
  path->rLineTo(x, y);
}

SKWASM_EXPORT void path_quadraticBezierTo(Path* path,
                                          Scalar x1,
                                          Scalar y1,
                                          Scalar x2,
                                          Scalar y2) {
  path->quadTo(x1, y1, x2, y2);
}

SKWASM_EXPORT void path_relativeQuadraticBezierTo(Path* path,
                                                  Scalar x1,
                                                  Scalar y1,
                                                  Scalar x2,
                                                  Scalar y2) {
  path->rQuadTo(x1, y1, x2, y2);
}

SKWASM_EXPORT void path_cubicTo(Path* path,
                                Scalar x1,
                                Scalar y1,
                                Scalar x2,
                                Scalar y2,
                                Scalar x3,
                                Scalar y3) {
  path->cubicTo(x1, y1, x2, y2, x3, y3);
}

SKWASM_EXPORT void path_relativeCubicTo(Path* path,
                                        Scalar x1,
                                        Scalar y1,
                                        Scalar x2,
                                        Scalar y2,
                                        Scalar x3,
                                        Scalar y3) {
  path->rCubicTo(x1, y1, x2, y2, x3, y3);
}

SKWASM_EXPORT void path_conicTo(Path* path,
                                Scalar x1,
                                Scalar y1,
                                Scalar x2,
                                Scalar y2,
                                Scalar w) {
  path->conicTo(x1, y1, x2, y2, w);
}

SKWASM_EXPORT void path_relativeConicTo(Path* path,
                                        Scalar x1,
                                        Scalar y1,
                                        Scalar x2,
                                        Scalar y2,
                                        Scalar w) {
  path->rConicTo(x1, y1, x2, y2, w);
}

SKWASM_EXPORT void path_arcToOval(Path* path,
                                  const Rect* rect,
                                  Scalar startAngle,
                                  Scalar sweepAngle,
                                  bool forceMoveTo) {
  path->arcTo(*rect, startAngle, sweepAngle, forceMoveTo);
}

SKWASM_EXPORT void path_arcToRotated(Path* path,
                                     Scalar rx,
                                     Scalar ry,
                                     Scalar xAxisRotate,
                                     Path::ArcSize arcSize,
                                     PathDirection pathDirection,
                                     Scalar x,
                                     Scalar y) {
  path->arcTo(rx, ry, xAxisRotate, arcSize, pathDirection, x, y);
}

SKWASM_EXPORT void path_relativeArcToRotated(Path* path,
                                             Scalar rx,
                                             Scalar ry,
                                             Scalar xAxisRotate,
                                             Path::ArcSize arcSize,
                                             PathDirection pathDirection,
                                             Scalar x,
                                             Scalar y) {
  path->rArcTo(rx, ry, xAxisRotate, arcSize, pathDirection, x, y);
}

SKWASM_EXPORT void path_addRect(Path* path, const Rect* rect) {
  path->addRect(*rect);
}

SKWASM_EXPORT void path_addOval(Path* path, const Rect* oval) {
  path->addOval(*oval, PathDirection::kCW, 1);
}

SKWASM_EXPORT void path_addArc(Path* path,
                               const Rect* oval,
                               Scalar startAngle,
                               Scalar sweepAngle) {
  path->addArc(*oval, startAngle, sweepAngle);
}

SKWASM_EXPORT void path_addPolygon(Path* path,
                                   const Point* points,
                                   int count,
                                   bool close) {
  path->addPoly(points, count, close);
}

SKWASM_EXPORT void path_addRRect(Path* path, const Scalar* rrectValues) {
  path->addRRect(createRRect(rrectValues), PathDirection::kCW);
}

SKWASM_EXPORT void path_addPath(Path* path,
                                const Path* other,
                                const Scalar* matrix33,
                                Path::AddPathMode extendPath) {
  path->addPath(*other, createMatrix(matrix33), extendPath);
}

SKWASM_EXPORT void path_close(Path* path) {
  path->close();
}

SKWASM_EXPORT void path_reset(Path* path) {
  path->reset();
}

SKWASM_EXPORT bool path_contains(Path* path, Scalar x, Scalar y) {
  return path->contains(x, y);
}

SKWASM_EXPORT void path_transform(Path* path, const Scalar* matrix33) {
  path->transform(createMatrix(matrix33));
}

SKWASM_EXPORT void path_getBounds(Path* path, Rect* rect) {
  *rect = path->getBounds();
}

SKWASM_EXPORT Path* path_combine(PathOp operation,
                                 const Path* path1,
                                 const Path* path2) {
  Path* output = new Path();
  if (Op(*path1, *path2, operation, output)) {
    output->setFillType(path1->getFillType());
    return output;
  } else {
    delete output;
    return nullptr;
  }
}
