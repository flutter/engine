// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRING_UTILS_H_
#define BASE_STRING_UTILS_H_

#include <string>

#include "ax_build/build_config.h"

namespace base {

// Return a C++ string given printf-like input.
template <typename... Args>
std::string StringPrintf(const std::string& format, Args... args) {
  // Calculate the buffer size.
  int size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);
}

std::u16string ASCIIToUTF16(std::string src);

}  // namespace base

#endif  // BASE_STRING_UTILS_H_
