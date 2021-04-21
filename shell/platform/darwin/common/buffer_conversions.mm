// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/buffer_conversions.h"

namespace flutter {

std::vector<uint8_t> CopyNSDataToVector(NSData* data) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.bytes);
  return std::vector<uint8_t>(bytes, bytes + data.length);
}

std::unique_ptr<fml::Mapping> CopyNSDataToMapping(NSData* data) {
  return std::make_unique<fml::DataMapping>(CopyNSDataToVector(data));
}

}  // namespace flutter
