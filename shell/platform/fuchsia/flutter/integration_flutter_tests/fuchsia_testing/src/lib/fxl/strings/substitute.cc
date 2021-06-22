// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/substitute.h"

#include <array>
#include <cassert>
#include <iostream>
#include <tuple>
#include <utility>

// Macro for aborting only if we are in debug mode.
#ifndef NDEBUG
#define ABORT_DEBUG() abort()
#else
#define ABORT_DEBUG()
#endif

namespace fxl {

static std::string SubstituteWithArray(std::string_view format, std::string_view* args,
                                       size_t nargs);

std::string Substitute(std::string_view format, std::string_view arg0) {
  std::array<std::string_view, 1> arr = {arg0};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1) {
  std::array<std::string_view, 2> arr = {arg0, arg1};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2) {
  std::array<std::string_view, 3> arr = {arg0, arg1, arg2};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3) {
  std::array<std::string_view, 4> arr = {arg0, arg1, arg2, arg3};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4) {
  std::array<std::string_view, 5> arr = {arg0, arg1, arg2, arg3, arg4};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5) {
  std::array<std::string_view, 6> arr = {arg0, arg1, arg2, arg3, arg4, arg5};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6) {
  std::array<std::string_view, 7> arr = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6, std::string_view arg7) {
  std::array<std::string_view, 8> arr = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6, std::string_view arg7,
                       std::string_view arg8) {
  std::array<std::string_view, 9> arr = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format, std::string_view arg0, std::string_view arg1,
                       std::string_view arg2, std::string_view arg3, std::string_view arg4,
                       std::string_view arg5, std::string_view arg6, std::string_view arg7,
                       std::string_view arg8, std::string_view arg9) {
  std::array<std::string_view, 10> arr = {arg0, arg1, arg2, arg3, arg4,
                                          arg5, arg6, arg7, arg8, arg9};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

enum class CharType { kPositionalId, kMissingId, kRegularChar, kDollar };

// Returns the type of character, and positional id if type is kPositionalId.
inline static std::pair<CharType, size_t> GetCharInfo(std::string_view str, size_t pos) {
  if (str[pos] == '$' && pos < str.size() - 1 && str[pos + 1] >= '0' && str[pos + 1] <= '9') {
    return {CharType::kPositionalId, str[pos + 1] - '0'};
  }
  if (pos == str.size() - 1 && str[pos] == '$') {
    return {CharType::kMissingId, -1};
  }
  if (pos < str.size() - 1 && str[pos] == '$' && str[pos + 1] == '$') {
    return {CharType::kDollar, -1};
  }
  return {CharType::kRegularChar, -1};
}

std::string SubstituteWithArray(std::string_view format, std::string_view* args, size_t nargs) {
  static constexpr size_t kMaxArgs = 10;
  if (nargs > kMaxArgs) {
    std::cerr << "More than " << kMaxArgs << "args: " << nargs << std::endl;
    abort();
  };

  int out_size = 0;
  size_t pos = 0;
  while (pos < format.size()) {
    CharType char_type;
    size_t positional_id;
    std::tie(char_type, positional_id) = GetCharInfo(format, pos);
    switch (char_type) {
      case CharType::kPositionalId:
        if (positional_id >= nargs) {
          // Error: missing argument.
          std::cerr << "fxl::Substitute missing argument for $" << positional_id << ": \"" << format
                    << "\"" << std::endl;
          ABORT_DEBUG();
          return "";
        }
        out_size += args[positional_id].size();
        pos += 2;
        break;
      case CharType::kMissingId:
        std::cerr << "fxl::Substitute encountered trailing '$': \"" << format << "\"" << std::endl;
        ABORT_DEBUG();
        return "";
      case CharType::kRegularChar:
        ++out_size;
        ++pos;
        break;
      case CharType::kDollar:
        ++out_size;
        pos += 2;
        break;
    }
  }

  std::string output;
  output.resize(out_size);
  size_t format_pos = 0, out_pos = 0;
  while (format_pos < format.size()) {
    CharType char_type;
    size_t positional_id;
    std::tie(char_type, positional_id) = GetCharInfo(format, format_pos);
    switch (char_type) {
      case CharType::kPositionalId: {
        const auto& arg = args[positional_id];
        output.replace(out_pos, arg.size(), arg.data(), arg.size());
        format_pos += 2;
        out_pos += arg.size();
        break;
      }
      case CharType::kMissingId:
        abort();
      case CharType::kRegularChar:
        output[out_pos++] = format[format_pos++];
        break;
      case CharType::kDollar:
        output[out_pos++] = '$';
        format_pos += 2;
        break;
    }
  }
  return output;
}

}  // namespace fxl
