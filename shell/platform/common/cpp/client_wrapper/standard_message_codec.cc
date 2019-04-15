// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/standard_message_codec.h"

#include <assert.h>
#include <cstring>
#include <iostream>
#include <string>

#include "byte_stream_wrappers.h"
#include "standard_codec_serializer.h"

namespace flutter {

// static
const StandardMessageCodec& StandardMessageCodec::GetInstance() {
  static StandardMessageCodec sInstance;
  return sInstance;
}

StandardMessageCodec::StandardMessageCodec() = default;

StandardMessageCodec::~StandardMessageCodec() = default;

std::unique_ptr<EncodableValue> StandardMessageCodec::DecodeMessageInternal(
    const uint8_t* binary_message,
    const size_t message_size) const {
  StandardCodecSerializer serializer;
  ByteBufferStreamReader stream(binary_message, message_size);
  return std::make_unique<EncodableValue>(serializer.ReadValue(&stream));
}

std::unique_ptr<std::vector<uint8_t>>
StandardMessageCodec::EncodeMessageInternal(
    const EncodableValue& message) const {
  StandardCodecSerializer serializer;
  auto encoded = std::make_unique<std::vector<uint8_t>>();
  ByteBufferStreamWriter stream(encoded.get());
  serializer.WriteValue(message, &stream);
  return encoded;
}

}  // namespace flutter
