// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_LIGHTWEIGHT_STRING_H_
#define FLUTTER_FML_LIGHTWEIGHT_STRING_H_

#include <cstdlib>
#include <cstring>
#include <string>

#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"

namespace fml {

/// An 16 byte string that does not copy string literals.
class LightweightString {
 public:
  LightweightString() {
    str_.c = nullptr;
    is_owned_ = false;
  }

  LightweightString(const LightweightString& str) {
    if (str.is_owned_) {
      str_.c = strdup(str.str_.cc);
      is_owned_ = true;
    } else {
      str_.cc = str.str_.cc;
      is_owned_ = false;
    }
  }

  template <size_t N>
  constexpr LightweightString(const char (&s)[N]) {
    str_.cc = s;
    is_owned_ = false;
  }

  LightweightString(const std::string& str) {
    str_.c = strdup(str.c_str());
    is_owned_ = true;
  }

  ~LightweightString() {
    if (is_owned_) {
      free(str_.c);
    }
  }

  const char* c_str() const { return str_.cc; }

  bool empty() const { return str_.cc == nullptr; }

  LightweightString& operator=(const LightweightString& str) {
    if (is_owned_) {
      free(str_.c);
    }
    if (str.is_owned_) {
      str_.c = strdup(str.str_.cc);
      is_owned_ = true;
    } else {
      str_.cc = str.str_.cc;
      is_owned_ = false;
    }
    return *this;
  }

 private:
  typedef union {
    char* c;
    const char* cc;
  } StrPtr;

  StrPtr str_;
  uint8_t is_owned_;
};

static_assert(sizeof(LightweightString) < sizeof(std::string));

}  // namespace fml

#endif  // FLUTTER_FML_LIGHTWEIGHT_STRING_H_
