// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/testing/views/color.h"

#include <lib/syslog/cpp/macros.h>

#include "src/lib/fsl/vmo/vector.h"
#include "src/lib/fxl/strings/string_printf.h"

namespace scenic {

// RGBA hex dump
std::ostream& operator<<(std::ostream& os, const Color& c) {
  return os << fxl::StringPrintf("%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
}

Screenshot::Screenshot(const fuchsia::ui::scenic::ScreenshotData& screenshot_data)
    : width_(screenshot_data.info.width), height_(screenshot_data.info.height) {
  FX_CHECK(screenshot_data.info.pixel_format == fuchsia::images::PixelFormat::BGRA_8)
      << "Non-BGRA_8 pixel formats not supported";
  FX_CHECK(fsl::VectorFromVmo(screenshot_data.data, &data_)) << "Failed to read screenshot";
}

const Color* Screenshot::operator[](size_t row) const { return &begin()[row * width_]; }

const Color& Screenshot::ColorAt(float x, float y) const {
  FX_CHECK(x >= 0 && x < 1 && y >= 0 && y < 1)
      << "(" << x << ", " << y << ") is out of bounds [0, 1) x [0, 1)";
  const size_t ix = static_cast<size_t>(x * static_cast<float>(width_));
  const size_t iy = static_cast<size_t>(y * static_cast<float>(height_));
  return (*this)[iy][ix];
}

const Color* Screenshot::begin() const { return reinterpret_cast<const Color*>(data_.data()); }

const Color* Screenshot::end() const { return &begin()[width_ * height_]; }

std::map<Color, size_t> Screenshot::Histogram() const {
  std::map<Color, size_t> histogram;

  for (const auto color : *this) {
    ++histogram[color];
  }

  return histogram;
}

}  // namespace scenic
