// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/golden_tests/libpng_screenshot.h"

namespace impeller {
namespace testing {

LibPNGScreenshot::LibPNGScreenshot(const uint8_t* bytes,
                                   size_t width,
                                   size_t height)
    : width_(width), height_(height) {}

LibPNGScreenshot::~LibPNGScreenshot() = default;

const uint8_t* LibPNGScreenshot::GetBytes() const {
  return bytes_.data();
}

size_t LibPNGScreenshot::GetHeight() const {
  return height_;
}

size_t LibPNGScreenshot::GetWidth() const {
  return width_;
}

size_t LibPNGScreenshot::GetBytesPerRow() const {
  return 4 * width_;
}

bool LibPNGScreenshot::WriteToPNG(const std::string& path) const {
  return false;
}

}  // namespace testing
}  // namespace impeller
