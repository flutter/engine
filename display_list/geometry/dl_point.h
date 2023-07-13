// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"

namespace flutter {

template <typename T>
struct DlPointT {
  T x;
  T y;

  DlPointT() : DlPointT(0, 0) {}
  DlPointT(T x, T y) : x(x), y(y) {}
};

using DlPointF = DlPointT<DlScalar>;
using DlPointI = DlPointT<DlInt>;

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
