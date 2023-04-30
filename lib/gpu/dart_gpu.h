// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_GPU_DART_GPU_H_
#define FLUTTER_LIB_GPU_DART_GPU_H_

#include "flutter/common/settings.h"
#include "flutter/fml/macros.h"

namespace flutter {

class DartGPU {
 public:
  static void InitForIsolate(const Settings& settings);

 private:
  FML_DISALLOW_IMPLICIT_CONSTRUCTORS(DartGPU);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_GPU_DART_UI_H_
