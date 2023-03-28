// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/geometry/half.h"

#include "flutter/fml/logging.h"

namespace impeller {

InternalHalf ScalarToHalf(Scalar f) {
#ifdef FML_OS_WIN
  FML_UNREACHABLE();
#endif
  return static_cast<InternalHalf>(f);
}

}  // namespace impeller
