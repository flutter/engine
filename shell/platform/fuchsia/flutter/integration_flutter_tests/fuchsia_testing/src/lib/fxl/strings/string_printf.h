// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// |printf()|-like formatting functions that output/append to C++ strings.

#ifndef SRC_LIB_FXL_STRINGS_STRING_PRINTF_H_
#define SRC_LIB_FXL_STRINGS_STRING_PRINTF_H_

#include <stdarg.h>

#include <string>

#include "src/lib/fxl/fxl_export.h"
#include "src/lib/fxl/macros.h"

namespace fxl {

// Formats |printf()|-like input and returns it as an |std::string|.
[[nodiscard, gnu::format(printf, 1, 2)]] std::string StringPrintf(const char* format, ...);

// Formats |vprintf()|-like input and returns it as an |std::string|.
[[nodiscard]] std::string StringVPrintf(const char* format, va_list ap);

// Formats |printf()|-like input and appends it to |*dest|.
[[gnu::format(printf, 2, 3)]] void StringAppendf(std::string* dest, const char* format, ...);

// Formats |vprintf()|-like input and appends it to |*dest|.
void StringVAppendf(std::string* dest, const char* format, va_list ap);

}  // namespace fxl

#endif  // SRC_LIB_FXL_STRINGS_STRING_PRINTF_H_
