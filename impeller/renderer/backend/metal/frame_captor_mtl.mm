// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/frame_captor_mtl.h"

namespace impeller {

FrameCaptorMTL::FrameCaptorMTL(id<MTLDevice> device) : device_(device) {}

FrameCaptorMTL::~FrameCaptorMTL() = default;

bool FrameCaptorMTL::StartCapturingFrame(
    FrameCaptorConfiguration configuration) {
  if (!device_) {
    return false;
  }

  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
  if (captureManager.isCapturing) {
    return false;
  }

  if (@available(iOS 13.0, macOS 10.15, *)) {
    MTLCaptureDescriptor* desc = [[MTLCaptureDescriptor alloc] init];
    desc.captureObject = device_;
    return [captureManager startCaptureWithDescriptor:desc error:nil];
  }

  [captureManager startCaptureWithDevice:device_];
  return captureManager.isCapturing;
}

bool FrameCaptorMTL::StopCapturing() {
  if (!device_) {
    return false;
  }

  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
  if (!captureManager.isCapturing) {
    return false;
  }

  [captureManager stopCapture];
  return !captureManager.isCapturing;
}

}  // namespace impeller
