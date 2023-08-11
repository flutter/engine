// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_angle.h"

namespace flutter {

DlFVector DlAngle::CosSin() const {
  DlScalar r = radians();
  DlScalar c = cosf(r);
  DlScalar s;
  if (c == -1.0f || c == 1.0f) {
    s = 0.0f;
  } else {
    s = sinf(r);
    if (s == -1.0f || s == 1.0f) {
      c = 0.0f;
    }
  }
  return {c, s};
}

}  // namespace flutter
