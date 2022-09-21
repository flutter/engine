// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/backtrace.h"

namespace fml {

void InstallCrashHandler() {
  // Not supported.
}

bool IsCrashHandlingSupported() {
  return false;
}

}  // namespace fml
