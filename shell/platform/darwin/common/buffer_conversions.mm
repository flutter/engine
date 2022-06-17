// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/buffer_conversions.h"

#include "flutter/fml/platform/darwin/scoped_nsobject.h"

namespace flutter {
namespace {
class NSDataMapping : public fml::Mapping {
 public:
  explicit NSDataMapping(NSData* data) : data_([data retain]) {}

  size_t GetSize() const override { return [data_.get() length]; }

  const uint8_t* GetMapping() const override {
    return static_cast<const uint8_t*>([data_.get() bytes]);
  }

  uint8_t* GetMutableMapping() const override {
    // TODO(tbd): Implement this so ownership can be passed to Dart.
    // Previously this was attempted with a const_cast of the mapping.  That is
    // safe in most cases since the data is ephemeral and we are the creaters
    // and the sole owners of the data. That is not the case when the
    // BinaryCodec is used however.
    return nullptr;
  }

  bool IsDontNeedSafe() const override { return false; }

 private:
  fml::scoped_nsobject<NSData> data_;
  FML_DISALLOW_COPY_AND_ASSIGN(NSDataMapping);
};
}  // namespace

fml::MallocMapping CopyNSDataToMapping(NSData* data) {
  const uint8_t* bytes = static_cast<const uint8_t*>(data.bytes);
  return fml::MallocMapping::Copy(bytes, data.length);
}

NSData* ConvertMappingToNSData(fml::MallocMapping buffer) {
  size_t size = buffer.GetSize();
  return [NSData dataWithBytesNoCopy:buffer.Release() length:size];
}

std::unique_ptr<fml::Mapping> ConvertNSDataToMappingPtr(NSData* data) {
  return std::make_unique<NSDataMapping>(data);
}

NSData* CopyMappingPtrToNSData(std::unique_ptr<fml::Mapping> mapping) {
  return [NSData dataWithBytes:mapping->GetMapping() length:mapping->GetSize()];
}

}  // namespace flutter
