// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "half.h"

namespace impeller {

_Float16 ScalarToHalf(Scalar f) {
  return static_cast<_Float16>(f);
}

}  // namespace impeller
