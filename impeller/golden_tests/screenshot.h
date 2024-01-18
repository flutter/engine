// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GOLDEN_TESTS_SCREENSHOT_H_
#define FLUTTER_IMPELLER_GOLDEN_TESTS_SCREENSHOT_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace impeller {
namespace testing {

class Screenshot {
 public:
  virtual ~Screenshot() = default;

  virtual const uint8_t* GetBytes() const = 0;

  virtual size_t GetHeight() const = 0;

  virtual size_t GetWidth() const = 0;

  virtual bool WriteToPNG(const std::string& path) const = 0;
};

}  // namespace testing
}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GOLDEN_TESTS_SCREENSHOT_H_
