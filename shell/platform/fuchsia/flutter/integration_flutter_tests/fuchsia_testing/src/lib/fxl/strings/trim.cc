// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/trim.h"

namespace fxl {

std::string_view TrimString(std::string_view str, std::string_view chars_to_trim) {
  size_t start_index = str.find_first_not_of(chars_to_trim);
  if (start_index == std::string_view::npos) {
    return std::string_view();
  }
  size_t end_index = str.find_last_not_of(chars_to_trim);
  return str.substr(start_index, end_index - start_index + 1);
}

}  // namespace fxl
