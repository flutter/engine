// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FXL_STRINGS_SPLIT_STRING_H_
#define SRC_LIB_FXL_STRINGS_SPLIT_STRING_H_

#include <string>
#include <string_view>
#include <vector>

#include "src/lib/fxl/fxl_export.h"

namespace fxl {

enum WhiteSpaceHandling {
  kKeepWhitespace,
  kTrimWhitespace,
};

enum SplitResult {
  // Strictly return all results.
  kSplitWantAll,

  // Only nonempty results will be added to the results.
  kSplitWantNonEmpty,
};

// Split the given string on ANY of the given separators, returning copies of
// the result
std::vector<std::string> SplitStringCopy(std::string_view input, std::string_view separators,
                                         WhiteSpaceHandling whitespace, SplitResult result_type);

// Like SplitStringCopy above except it returns a vector of std::string_views which
// reference the original buffer without copying.
std::vector<std::string_view> SplitString(std::string_view input, std::string_view separators,
                                          WhiteSpaceHandling whitespace, SplitResult result_type);

}  // namespace fxl

#endif  // SRC_LIB_FXL_STRINGS_SPLIT_STRING_H_
