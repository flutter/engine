// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_TEXT_EDITING_DELTA_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_TEXT_EDITING_DELTA_H_

#include <codecvt>
#include <string>

#include "flutter/shell/platform/common/text_range.h"

namespace flutter {

struct TextEditingDelta {
 public:
  TextEditingDelta(std::u16string textBeforeChange,
                   TextRange range,
                   std::u16string text);
  virtual ~TextEditingDelta();

  std::string oldText() const { return utf16ToUtf8(oldText_); }
  std::string deltaText() const { return utf16ToUtf8(deltaText_); }
  int deltaStart() const { return deltaStart_; }
  int deltaEnd() const { return deltaEnd_; }

  TextEditingDelta(std::string textBeforeChange,
                   TextRange range,
                   std::string text);
  TextEditingDelta(std::u16string text);
  TextEditingDelta(std::string text);

  bool operator==(const TextEditingDelta& rhs) const {
    return oldText_ == rhs.oldText_ && deltaText_ == rhs.deltaText_ &&
           deltaStart_ == rhs.deltaStart_ && deltaEnd_ == rhs.deltaEnd_;
  }

  bool operator!=(const TextEditingDelta& rhs) const { return !(*this == rhs); }

 private:
  std::u16string oldText_;
  std::u16string deltaText_;
  int deltaStart_;
  int deltaEnd_;

  void setDeltas(std::u16string oldText,
                 std::u16string newTxt,
                 int newStart,
                 int newEnd) {
    oldText_ = oldText;
    deltaText_ = newTxt;
    deltaStart_ = newStart;
    deltaEnd_ = newEnd;
  }

  std::string utf16ToUtf8(std::u16string string) const {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>
        utf8_converter;
    return utf8_converter.to_bytes(string);
  }

  std::u16string utf8ToUtf16(std::string string) const {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>
        utf16_converter;
    return utf16_converter.from_bytes(string);
  }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_TEXT_EDITING_DELTA_H_
