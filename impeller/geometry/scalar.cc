// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scalar.h"

namespace impeller {

Half ScalarToHalf(Scalar f) {
  __fp16 storage = f;
  return storage;
}

}  // namespace impeller
