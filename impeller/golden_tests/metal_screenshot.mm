// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/golden_tests/metal_screenshot.h"

namespace impeller {
namespace testing {

MetalScreenshot::MetalScreenshot(CGImageRef cgImage) : cgImage_(cgImage) {
  CGDataProviderRef data_provider = CGImageGetDataProvider(cgImage);
  pixel_data_ = CGDataProviderCopyData(data_provider);
}

MetalScreenshot::~MetalScreenshot() {
  if (pixel_data_) {
    // CFRelease is documented to not accept a null pointer
    CFRelease(pixel_data_);
  }
  // CGImageRelease is documented to accept a null pointer
  CGImageRelease(cgImage_);
}

const UInt8* MetalScreenshot::GetBytes() const {
  // CFDataGetBytePtr will crash on a null pointer
  return pixel_data_ ? CFDataGetBytePtr(pixel_data_) : nullptr;
}

size_t MetalScreenshot::GetHeight() const {
  // CGImageGetHeight actually returns 0 for a null image,
  // but that is not documented
  return cgImage_ ? CGImageGetHeight(cgImage_) : 0;
}

size_t MetalScreenshot::GetWidth() const {
  // CGImageGetWidth actually returns 0 for a null image,
  // but that is not documented
  return cgImage_ ? CGImageGetWidth(cgImage_) : 0;
}

bool MetalScreenshot::WriteToPNG(const std::string& path) const {
  bool result = false;
  if (cgImage_) {
    NSURL* output_url =
        [NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()]];
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
        (__bridge CFURLRef)output_url, kUTTypePNG, 1, nullptr);
    if (destination != nullptr) {
      CGImageDestinationAddImage(destination, cgImage_,
                                 (__bridge CFDictionaryRef) @{});

      if (CGImageDestinationFinalize(destination)) {
        result = true;
      }

      CFRelease(destination);
    }
  }
  return result;
}

}  // namespace testing
}  // namespace impeller
