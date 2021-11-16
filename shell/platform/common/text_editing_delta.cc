// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/text_editing_delta.h"

namespace flutter {

TextEditingDelta::~TextEditingDelta() = default;

TextEditingDelta::TextEditingDelta(std::u16string textBeforeChange,
                                   TextRange range,
                                   std::u16string text) {
  int start = range.start();
  int end = range.start() + range.length();
  setDeltas(textBeforeChange, text, start, end);
}

TextEditingDelta::TextEditingDelta(std::string textBeforeChange,
                                   TextRange range,
                                   std::string text) {
  int start = range.start();
  int end = range.start() + range.length();
  std::u16string textBeforeChange16 = utf8ToUtf16(textBeforeChange);
  std::u16string text16 = utf8ToUtf16(text);
  setDeltas(textBeforeChange16, text16, start, end);
}

TextEditingDelta::TextEditingDelta(std::u16string text) {
  setDeltas(text, u"", -1, -1);
}

TextEditingDelta::TextEditingDelta(std::string text) {
  setDeltas(utf8ToUtf16(text), u"", -1, -1);
}

}  // namespace flutter
