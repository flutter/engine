// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTNIG_IMMUTABLE_BUFER_H_
#define FLUTTER_LIB_UI_PAINTNIG_IMMUTABLE_BUFER_H_

#include <cstdint>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace flutter {
class ImmutableBuffer : public RefCountedDartWrappable<ImmutableBuffer> {
 public:
  ~ImmutableBuffer() override = default;

  static void init(Dart_NativeArguments args);

  int length() const {
    FML_DCHECK(data_);
    return data_->size();
  }

  int elementAt(size_t index) const {
    FML_DCHECK(data_);
    FML_DCHECK(index < data_->size());
    return data_->bytes()[index];
  }

  sk_sp<SkData> data() const { return data_; }

  void dispose() {
    ClearDartWrapper();
    data_.reset();
  }

  size_t GetAllocationSize() const override {
    return sizeof(ImmutableBuffer) + (data_->size() * 4);
  }

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  explicit ImmutableBuffer(sk_sp<SkData> data) : data_(std::move(data)) {}

  sk_sp<SkData> data_;

  static sk_sp<SkData> MakeSkDataWithCopy(const void* data, size_t length);

  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(ImmutableBuffer);
  FML_DISALLOW_COPY_AND_ASSIGN(ImmutableBuffer);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTNIG_IMMUTABLE_BUFER_H_
