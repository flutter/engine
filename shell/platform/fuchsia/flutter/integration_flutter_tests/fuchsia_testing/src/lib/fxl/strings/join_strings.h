// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FXL_STRINGS_JOIN_STRINGS_H_
#define LIB_FXL_STRINGS_JOIN_STRINGS_H_

#include <string>
#include <vector>

namespace fxl {

// Join a container of strings with a separator. This is expected to work with
// std::vector<std::string> and std::array<std::string> but will work with any
// container class that supports iterators and basic capacity access (ie:
// size()) as defined in the Containers library (see:
// http://en.cppreference.com/w/cpp/container).
template <typename StringContainer>
std::string JoinStrings(const StringContainer& strings, const std::string& separator = "") {
  size_t output_length = 0;
  // Add up the sizes of the strings.
  for (const auto& i : strings) {
    output_length += i.size();
  }
  // Add the sizes of the separators.
  if (!strings.empty()) {
    output_length += (strings.size() - 1) * separator.size();
  }

  std::string joined;
  joined.reserve(output_length);

  bool first = true;
  for (const auto& i : strings) {
    if (!first) {
      joined.append(separator);
    } else {
      first = false;
    }
    joined.append(i.begin(), i.end());
  }

  return joined;
}

}  // namespace fxl

#endif  // LIB_FXL_STRINGS_JOIN_STRINGS_H_
