// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/string_message_codec.h"

namespace flutter {

// static
const StringMessageCodec& StringMessageCodec::GetInstance() {
  static StringMessageCodec sInstance;
  return sInstance;
}

std::unique_ptr<std::string> StringMessageCodec::DecodeMessageInternal(
    const uint8_t* binary_message,
    const size_t message_size) const {
  return std::make_unique<std::string>(
      reinterpret_cast<const char*>(binary_message), message_size);
}

std::unique_ptr<std::vector<uint8_t>> StringMessageCodec::EncodeMessageInternal(
    const std::string& message) const {
  std::vector<uint8_t> string_message(message.length());
  std::copy(message.begin(), message.end(), string_message.begin());

  return std::make_unique<std::vector<uint8_t>>(string_message);
}

}  // namespace flutter
