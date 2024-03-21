// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_SKIA_ALIASES_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_SKIA_ALIASES_H_

#include "third_party/skia/include/core/SkM44.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"

using DlScalar = SkScalar;
using DlIPoint = SkIPoint;
using DlPoint = SkPoint;
using DlIRect = SkIRect;
using DlRect = SkRect;
using DlTransform3x3 = SkMatrix;
using DlTransform4x4 = SkM44;

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_SKIA_ALIASES_H_
