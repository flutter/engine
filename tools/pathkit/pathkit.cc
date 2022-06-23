// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pathkit.h"

#include "third_party/skia/include/core/SkString.h"
#include "third_party/skia/include/utils/SkParsePath.h"

namespace flutter {
SkPath* CreatePath(SkPathFillType fill_type) {
  auto* path = new SkPath();
  path->setFillType(fill_type);
  return path;
}

void DestroyPath(SkPath* path) {
  delete path;
}

void MoveTo(SkPath* path, SkScalar x, SkScalar y) {
  path->moveTo(x, y);
}

void LineTo(SkPath* path, SkScalar x, SkScalar y) {
  path->lineTo(x, y);
}

void CubicTo(SkPath* path,
             SkScalar x1,
             SkScalar y1,
             SkScalar x2,
             SkScalar y2,
             SkScalar x3,
             SkScalar y3) {
  path->cubicTo(x1, y1, x2, y2, x3, y3);
}

void Close(SkPath* path) {
  path->close();
}

void Reset(SkPath* path) {
  path->reset();
}

void dump(SkPath* path) {
  SkString string;
  SkParsePath::ToSVGString(*path, &string);
  printf("%s\n", string.c_str());
}

void Op(SkPath* one, SkPath* two, SkPathOp op) {
  Op(*one, *two, op, one);
}

}  // namespace flutter
