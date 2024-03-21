// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_IMPELLER_ADAPTERS_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_IMPELLER_ADAPTERS_H_

#include "flutter/impeller/geometry/matrix.h"
#include "flutter/impeller/geometry/point.h"
#include "flutter/impeller/geometry/rect.h"

using DlScalar = impeller::Scalar;

struct DlIPoint : public impeller::IPoint32 {};
struct DlPoint : public impeller::Point {};
struct DlRect : public impeller::Rect {};
struct DlIRect : public impeller::IRect {};
class DlTransform3x3 : public impeller::Matrix {};
class DlTransform4x4 : public impeller::Matrix {};

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_IMPELLER_ADAPTERS_H_
