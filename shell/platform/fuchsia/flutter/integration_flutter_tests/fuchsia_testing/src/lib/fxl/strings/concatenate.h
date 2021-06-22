// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FXL_STRINGS_CONCATENATE_H_
#define SRC_LIB_FXL_STRINGS_CONCATENATE_H_

#include <initializer_list>
#include <string>
#include <string_view>

#include "src/lib/fxl/fxl_export.h"

namespace fxl {

// Concatenates a fixed list of strings.
std::string Concatenate(std::initializer_list<std::string_view> string_views);

}  // namespace fxl

#endif  // SRC_LIB_FXL_STRINGS_CONCATENATE_H_
