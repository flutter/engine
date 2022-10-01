// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/frame_captor_mtl.h"

namespace impeller {

FrameCaptorMTL::FrameCaptorMTL(id<MTLDevice> device) : device_(device) {}

FrameCaptorMTL::~FrameCaptorMTL() = default;

bool FrameCaptorMTL::StartCapturingFrame(
    FrameCaptorConfiguration configuration) {
  return false;
}

bool FrameCaptorMTL::StopCapturing() {
  return false;
}

}  // namespace impeller
