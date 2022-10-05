// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frame_captor.h"

namespace impeller {

FrameCaptor::FrameCaptor() = default;

FrameCaptor::~FrameCaptor() = default;

bool FrameCaptor::StartCapturingFrame(FrameCaptorConfiguration configuration) {
  return false;
}

bool FrameCaptor::StopCapturingFrame() {
  return false;
}

}  // namespace impeller
