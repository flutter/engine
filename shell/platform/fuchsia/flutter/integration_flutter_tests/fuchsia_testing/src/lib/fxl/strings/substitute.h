// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FXL_STRINGS_SUBSTITUTE_H_
#define SRC_LIB_FXL_STRINGS_SUBSTITUTE_H_

#include <string>
#include <string_view>

#include "src/lib/fxl/fxl_export.h"

namespace fxl {

// Perform string substitutions using a positional notation.
//
// The format string uses positional identifiers consisting of a $ sign followed
// by a single digit: $0-$9. Each positional identifier refers to the
// corresponding string in the argument list: $0 for the first argument, etc.
// Unlike fxl::StringPrintf, callers do not have to specify the type, and
// it is possible to reuse the same positional identifier multiple times.
//
// If Substitute encounters an error (for example, not enough arguments), it
// crashes in debug mode, and returns an empty string in non-debug mode.
//
// This function is inspired by Abseil's strings/substitute.h.
std::string Substitute(std::string_view format, std::string_view arg0);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6, std::string_view arg7);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6, std::string_view arg7,
                       std::string_view arg8);
std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6, std::string_view arg7,
                       std::string_view arg8, std::string_view arg9);

}  // namespace fxl

#endif  // SRC_LIB_FXL_STRINGS_SUBSTITUTE_H_
