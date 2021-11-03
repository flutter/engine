// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_TEXT_EDITING_DELTA_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_TEXT_EDITING_DELTA_H_

#include <string>

#include "flutter/shell/platform/common/text_range.h"

namespace flutter {

class TextEditingDelta {
 public:
  TextEditingDelta(std::string textBeforeChange,
                   TextRange range,
                   std::string text);
  virtual ~TextEditingDelta();

  std::string oldText() const { return oldText_; }
  std::string deltaText() const { return deltaText_; }
  int deltaStart() const { return deltaStart_; }
  int deltaEnd() const { return deltaEnd_; }

  TextEditingDelta(std::string text);

 private:
  std::string oldText_;
  std::string deltaText_;
  int deltaStart_;
  int deltaEnd_;

  void setDeltas(std::string oldText,
                 std::string newTxt,
                 int newStart,
                 int newEnd) {
    oldText_ = oldText;
    deltaText_ = newTxt;
    deltaStart_ = newStart;
    deltaEnd_ = newEnd;
  }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_TEXT_EDITING_DELTA_H_
