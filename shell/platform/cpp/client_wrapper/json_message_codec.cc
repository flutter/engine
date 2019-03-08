// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/json_message_codec.h"

#include <iostream>
#include <string>

namespace flutter {

// static
const JsonMessageCodec &JsonMessageCodec::GetInstance() {
  static JsonMessageCodec sInstance;
  return sInstance;
}

std::unique_ptr<std::vector<uint8_t>> JsonMessageCodec::EncodeMessageInternal(
    const Json::Value &message) const {
  Json::StreamWriterBuilder writer_builder;
  std::string serialization = Json::writeString(writer_builder, message);

  return std::make_unique<std::vector<uint8_t>>(serialization.begin(),
                                                serialization.end());
}

std::unique_ptr<Json::Value> JsonMessageCodec::DecodeMessageInternal(
    const uint8_t *binary_message, const size_t message_size) const {
  Json::CharReaderBuilder reader_builder;
  std::unique_ptr<Json::CharReader> parser(reader_builder.newCharReader());

  auto raw_message = reinterpret_cast<const char *>(binary_message);
  auto json_message = std::make_unique<Json::Value>();
  std::string parse_errors;
  bool parsing_successful =
      parser->parse(raw_message, raw_message + message_size, json_message.get(),
                    &parse_errors);
  if (!parsing_successful) {
    std::cerr << "Unable to parse JSON message:" << std::endl
              << parse_errors << std::endl;
    return nullptr;
  }
  return json_message;
}

}  // namespace flutter
