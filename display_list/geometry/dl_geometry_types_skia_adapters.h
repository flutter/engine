// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_SKIA_ADAPTERS_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_SKIA_ADAPTERS_H_

#include "third_party/skia/include/core/SkM44.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"

using DlScalar = SkScalar;

// NOLINTBEGIN(google-explicit-constructor)

struct DlIPoint : public SkIPoint {
  constexpr DlIPoint() : SkIPoint() {}
  constexpr DlIPoint(const SkIPoint& sk_rect) : SkIPoint(sk_rect) {}
  constexpr DlIPoint(SkIPoint&& sk_rect) : SkIPoint(sk_rect) {}
};

struct DlPoint : public SkPoint {
  constexpr DlPoint() : SkPoint() {}
  constexpr DlPoint(const SkPoint& sk_rect) : SkPoint(sk_rect) {}
  constexpr DlPoint(SkPoint&& sk_rect) : SkPoint(sk_rect) {}
};

struct DlIRect : public SkIRect {
  constexpr DlIRect() : SkIRect() {}
  constexpr DlIRect(const SkIRect& sk_rect) : SkIRect(sk_rect) {}
  constexpr DlIRect(SkIRect&& sk_rect) : SkIRect(sk_rect) {}
};

struct DlRect : public SkRect {
  constexpr DlRect() : SkRect() {}
  constexpr DlRect(const SkRect& sk_rect) : SkRect(sk_rect) {}
  constexpr DlRect(SkRect&& sk_rect) : SkRect(sk_rect) {}
};

class DlTransform3x3 : public SkMatrix {
  constexpr DlTransform3x3() : SkMatrix() {}
  constexpr DlTransform3x3(const SkMatrix& sk_rect) : SkMatrix(sk_rect) {}
  constexpr DlTransform3x3(SkMatrix&& sk_rect) : SkMatrix(sk_rect) {}
};

class DlTransform4x4 : public SkM44 {
  constexpr DlTransform4x4() : SkM44() {}
  constexpr DlTransform4x4(const SkM44& sk_rect) : SkM44(sk_rect) {}
  constexpr DlTransform4x4(SkM44&& sk_rect) : SkM44(sk_rect) {}
};

// NOLINTEND(google-explicit-constructor)

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_SKIA_ADAPTERS_H_
