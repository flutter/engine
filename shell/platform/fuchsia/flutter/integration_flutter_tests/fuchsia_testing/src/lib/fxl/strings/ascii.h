// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FXL_STRINGS_ASCII_H_
#define SRC_LIB_FXL_STRINGS_ASCII_H_

#include <string>
#include <string_view>

#include "src/lib/fxl/fxl_export.h"

namespace fxl {

inline bool IsAsciiWhitespace(char c) { return c == ' ' || c == '\r' || c == '\n' || c == '\t'; }

inline char ToLowerASCII(char c) { return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c; }

inline char ToUpperASCII(char c) { return (c >= 'a' && c <= 'z') ? (c + static_cast<char>('A' - 'a')) : c; }

bool EqualsCaseInsensitiveASCII(std::string_view v1, std::string_view v2);

}  // namespace fxl

#endif  // SRC_LIB_FXL_STRINGS_ASCII_H_
