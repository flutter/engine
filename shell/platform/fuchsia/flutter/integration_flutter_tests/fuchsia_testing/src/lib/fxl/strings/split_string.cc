// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/split_string.h"

#include <string_view>

#include "src/lib/fxl/strings/trim.h"

namespace fxl {
namespace {

template <typename OutputType>
OutputType PieceToOutputType(std::string_view view) {
  return view;
}

template <>
std::string PieceToOutputType<std::string>(std::string_view view) {
  return std::string(view);
}

size_t FindFirstOf(std::string_view view, char c, size_t pos) { return view.find(c, pos); }

size_t FindFirstOf(std::string_view view, std::string_view one_of, size_t pos) {
  return view.find_first_of(one_of, pos);
}

template <typename Str, typename OutputStringType, typename DelimiterType>
std::vector<OutputStringType> SplitStringT(Str src, DelimiterType delimiter,
                                           WhiteSpaceHandling whitespace, SplitResult result_type) {
  std::vector<OutputStringType> result;
  if (src.empty())
    return result;

  size_t start = 0;
  while (start != Str::npos) {
    size_t end = FindFirstOf(src, delimiter, start);

    std::string_view view;
    if (end == Str::npos) {
      view = src.substr(start);
      start = Str::npos;
    } else {
      view = src.substr(start, end - start);
      start = end + 1;
    }
    if (whitespace == kTrimWhitespace) {
      view = TrimString(view, " \t\r\n");
    }
    if (result_type == kSplitWantAll || !view.empty()) {
      result.push_back(PieceToOutputType<OutputStringType>(view));
    }
  }
  return result;
}

}  // namespace

std::vector<std::string> SplitStringCopy(std::string_view input, std::string_view separators,
                                         WhiteSpaceHandling whitespace, SplitResult result_type) {
  if (separators.size() == 1) {
    return SplitStringT<std::string_view, std::string, char>(input, separators[0], whitespace,
                                                             result_type);
  }
  return SplitStringT<std::string_view, std::string, std::string_view>(input, separators,
                                                                       whitespace, result_type);
}
std::vector<std::string_view> SplitString(std::string_view input, std::string_view separators,
                                          WhiteSpaceHandling whitespace, SplitResult result_type) {
  if (separators.size() == 1) {
    return SplitStringT<std::string_view, std::string_view, char>(input, separators[0], whitespace,
                                                                  result_type);
  }
  return SplitStringT<std::string_view, std::string_view, std::string_view>(
      input, separators, whitespace, result_type);
}

}  // namespace fxl
