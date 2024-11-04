// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/golden_tests/libpng_screenshot.h"

#include <fstream>
#include "flutter/third_party/libpng/png.h"

namespace impeller {
namespace testing {

LibPNGScreenshot::LibPNGScreenshot(const uint8_t* bytes,
                                   size_t width,
                                   size_t height)
    : bytes_(bytes, bytes + 4 * width * height),
      width_(width),
      height_(height) {}

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
  std::ofstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png_ptr) {
    return false;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return false;
  }

  png_set_write_fn(
      png_ptr, (png_voidp)&file,
      [](png_structp png_ptr, png_bytep data, png_size_t length) {
        std::ofstream* file = (std::ofstream*)png_get_io_ptr(png_ptr);
        file->write(reinterpret_cast<char*>(data), length);
      },
      NULL);

  png_set_IHDR(png_ptr, info_ptr, width_, height_, 8, PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);

  png_set_bgr(png_ptr);

  for (size_t y = 0; y < height_; ++y) {
    png_write_row(png_ptr, &bytes_[y * width_ * 4]);
  }

  png_write_end(png_ptr, nullptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  return true;
}

}  // namespace testing
}  // namespace impeller
