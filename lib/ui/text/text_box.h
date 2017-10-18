// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_TEXT_TEXT_BOX_H_
#define FLUTTER_LIB_UI_TEXT_TEXT_BOX_H_

#include "dart/runtime/include/dart_api.h"
#include "lib/tonic/converter/dart_converter.h"
#include "third_party/skia/include/core/SkRect.h"

namespace tonic {

enum TextDirection { RTL, LTR };

class TextBox {
 public:
  TextBox() : is_null(true) {}
  TextBox(SkRect r, TextDirection direction)
      : sk_rect(std::move(r)), direction(direction), is_null(false) {}

  SkRect sk_rect;
  TextDirection direction;
  bool is_null;
};

template <>
struct DartConverter<TextBox> {
  static Dart_Handle ToDart(const TextBox& val);
};

}  // namespace tonic

#endif  // FLUTTER_LIB_UI_TEXT_TEXT_BOX_H_
