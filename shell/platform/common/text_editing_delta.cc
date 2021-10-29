// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/text_editing_delta.h"

namespace flutter {

//TextEditingDelta(std::string textBeforeChange, TextRange range, std::string text);
TextEditingDelta::TextEditingDelta(std::string textBeforeChange, TextRange range, std::string text) {
  int start = range.start();
  int end = range.start() + range.length();
  setDeltas(textBeforeChange, text, start, end);
}

TextEditingDelta::TextEditingDelta(std::string text) {
  setDeltas(text, "", -1, -1);
}

}  // namespace flutter
