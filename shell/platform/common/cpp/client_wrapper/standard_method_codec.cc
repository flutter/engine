// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/standard_method_codec.h"

#include <iostream>

#include "byte_stream_wrappers.h"
#include "standard_codec_serializer.h"

namespace flutter {

// static
const StandardMethodCodec& StandardMethodCodec::GetInstance() {
  static StandardMethodCodec sInstance;
  return sInstance;
}

std::unique_ptr<MethodCall<EncodableValue>>
StandardMethodCodec::DecodeMethodCallInternal(const uint8_t* message,
                                              const size_t message_size) const {
  StandardCodecSerializer serializer;
  ByteBufferStreamReader stream(message, message_size);
  EncodableValue method_name = serializer.ReadValue(&stream);
  if (!method_name.IsString()) {
    std::cerr << "Invalid method call; method name is not a string."
              << std::endl;
    return nullptr;
  }
  auto arguments =
      std::make_unique<EncodableValue>(serializer.ReadValue(&stream));
  return std::make_unique<MethodCall<EncodableValue>>(method_name.StringValue(),
                                                      std::move(arguments));
}

std::unique_ptr<std::vector<uint8_t>>
StandardMethodCodec::EncodeMethodCallInternal(
    const MethodCall<EncodableValue>& method_call) const {
  StandardCodecSerializer serializer;
  auto encoded = std::make_unique<std::vector<uint8_t>>();
  ByteBufferStreamWriter stream(encoded.get());
  serializer.WriteValue(EncodableValue(method_call.method_name()), &stream);
  if (method_call.arguments()) {
    serializer.WriteValue(*method_call.arguments(), &stream);
  } else {
    serializer.WriteValue(EncodableValue(), &stream);
  }
  return encoded;
}

std::unique_ptr<std::vector<uint8_t>>
StandardMethodCodec::EncodeSuccessEnvelopeInternal(
    const EncodableValue* result) const {
  StandardCodecSerializer serializer;
  auto encoded = std::make_unique<std::vector<uint8_t>>();
  ByteBufferStreamWriter stream(encoded.get());
  stream.WriteByte(0);
  if (result) {
    serializer.WriteValue(*result, &stream);
  } else {
    serializer.WriteValue(EncodableValue(), &stream);
  }
  return encoded;
}

std::unique_ptr<std::vector<uint8_t>>
StandardMethodCodec::EncodeErrorEnvelopeInternal(
    const std::string& error_code,
    const std::string& error_message,
    const EncodableValue* error_details) const {
  StandardCodecSerializer serializer;
  auto encoded = std::make_unique<std::vector<uint8_t>>();
  ByteBufferStreamWriter stream(encoded.get());
  stream.WriteByte(1);
  serializer.WriteValue(EncodableValue(error_code), &stream);
  if (error_message.empty()) {
    serializer.WriteValue(EncodableValue(), &stream);
  } else {
    serializer.WriteValue(EncodableValue(error_message), &stream);
  }
  if (error_details) {
    serializer.WriteValue(*error_details, &stream);
  } else {
    serializer.WriteValue(EncodableValue(), &stream);
  }
  return encoded;
}

}  // namespace flutter
