// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>

#include "flutter/display_list/dl_base_types.h"

namespace flutter {

bool DlScalars_AreAllFinite(const DlScalar* m, int count) {
  for (int i = 0; i < count; i++) {
    if (!DlScalar_IsFinite(m[i])) {
      return false;
    }
  }
  return true;
}

bool DlScalars_AreAllEqual(const DlScalar v1[], const DlScalar v2[],
                           int count) {
  return memcmp(v1, v2, sizeof(DlScalar) * count) == 0;
}

}  // namespace flutter
