// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPathBuilder.h"
#include "third_party/skia/include/core/SkPathTypes.h"
#include "third_party/skia/include/pathops/SkPathOps.h"

#ifdef _WIN32
#define API __declspec(dllexport)
#else
#define API __attribute__((visibility("default")))
#endif

extern "C" {

namespace flutter {

API SkPath* CreatePath(SkPathFillType fill_type);

API void DestroyPathBuilder(SkPath* path);

API void MoveTo(SkPath* path, SkScalar x, SkScalar y);

API void LineTo(SkPath* path, SkScalar x, SkScalar y);

API void CubicTo(SkPath* path,
                 SkScalar x1,
                 SkScalar y1,
                 SkScalar x2,
                 SkScalar y2,
                 SkScalar x3,
                 SkScalar y3);

API void Close(SkPath* path);

API void Reset(SkPath* path);

API void DestroyPath(SkPath* path);

API void Op(SkPath* one, SkPath* two, SkPathOp op);

API void dump(SkPath* path);

}  // namespace flutter
}
