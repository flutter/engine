// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/affinity.h"

namespace fml {

bool RequestAffinity(CpuAffinity affinity) {
  return false;
}

}  // namespace fml
