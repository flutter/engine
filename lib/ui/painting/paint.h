// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PAINT_H_
#define FLUTTER_LIB_UI_PAINTING_PAINT_H_

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/dl_op_flags.h"
#include "third_party/tonic/converter/dart_converter.h"

namespace flutter {

class Paint {
 public:
  Paint() = default;
  Paint(Dart_Handle paint_objects, void* paint_data);

  const DlPaint* paint(DlPaint& paint,
                       const DisplayListAttributeFlags& flags) const;

  void toDlPaint(DlPaint& paint) const;

  bool isNull() const { return paint_data_ == nullptr; }
  bool isNotNull() const { return paint_data_ != nullptr; }

 private:
  friend struct tonic::DartConverter<Paint>;

  Dart_Handle paint_objects_;
  void* paint_data_ = nullptr;
};

// The PaintData argument is a placeholder to receive encoded data for Paint
// objects. The data is actually processed by DartConverter<Paint>, which reads
// both at the given index and at the next index (which it assumes is a byte
// data for a Paint object).
class PaintData {};

}  // namespace flutter

namespace tonic {

template <>
struct DartConverter<flutter::Paint> {
  static flutter::Paint FromArguments(Dart_NativeArguments args,
                                      int index,
                                      Dart_Handle& exception);
};

template <>
struct DartConverter<flutter::PaintData> {
  static flutter::PaintData FromArguments(Dart_NativeArguments args,
                                          int index,
                                          Dart_Handle& exception);
};

}  // namespace tonic

#endif  // FLUTTER_LIB_UI_PAINTING_PAINT_H_
