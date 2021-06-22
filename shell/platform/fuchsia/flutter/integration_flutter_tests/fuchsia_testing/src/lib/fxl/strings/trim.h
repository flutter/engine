// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FXL_STRINGS_TRIM_H_
#define SRC_LIB_FXL_STRINGS_TRIM_H_

#include <string>
#include <string_view>

#include "src/lib/fxl/fxl_export.h"

namespace fxl {

// Returns a std::string_view over str, where chars_to_trim are removed from the
// beginning and end of the std::string_view.
std::string_view TrimString(std::string_view str, std::string_view chars_to_trim);

}  // namespace fxl

#endif  // SRC_LIB_FXL_STRINGS_TRIM_H_
