// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "string_utils.h"

#include <codecvt>
#include <locale>

namespace base {

std::u16string ASCIIToUTF16(std::string src) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.from_bytes(src);
}

}  // namespace base
