// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GOLDEN_TESTS_LIBPNG_SCREENSHOT_H_
#define FLUTTER_IMPELLER_GOLDEN_TESTS_LIBPNG_SCREENSHOT_H_

#include <cstdint>
#include <vector>
#include "flutter/impeller/golden_tests/screenshot.h"

namespace impeller {
namespace testing {

class LibPNGScreenshot : public Screenshot {
 public:
  explicit LibPNGScreenshot(const uint8_t* bytes, size_t width, size_t height);

  ~LibPNGScreenshot();

  const uint8_t* GetBytes() const override;

  size_t GetHeight() const override;

  size_t GetWidth() const override;

  size_t GetBytesPerRow() const override;

  bool WriteToPNG(const std::string& path) const override;

 private:
  std::vector<uint8_t> bytes_;
  size_t width_;
  size_t height_;
};

}  // namespace testing
}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GOLDEN_TESTS_LIBPNG_SCREENSHOT_H_