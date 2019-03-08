// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_JSON_METHOD_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_JSON_METHOD_CODEC_H_

#include <json/json.h>

#include "method_call.h"
#include "method_codec.h"

namespace flutter {

// An implementation of MethodCodec that uses JSON strings as the serialization.
class JsonMethodCodec : public MethodCodec<Json::Value> {
 public:
  // Returns the shared instance of the codec.
  static const JsonMethodCodec &GetInstance();

  ~JsonMethodCodec() = default;

  // Prevent copying.
  JsonMethodCodec(JsonMethodCodec const &) = delete;
  JsonMethodCodec &operator=(JsonMethodCodec const &) = delete;

 protected:
  // Instances should be obtained via GetInstance.
  JsonMethodCodec() = default;

  // MethodCodec:
  std::unique_ptr<MethodCall<Json::Value>> DecodeMethodCallInternal(
      const uint8_t *message, const size_t message_size) const override;
  std::unique_ptr<std::vector<uint8_t>> EncodeMethodCallInternal(
      const MethodCall<Json::Value> &method_call) const override;
  std::unique_ptr<std::vector<uint8_t>> EncodeSuccessEnvelopeInternal(
      const Json::Value *result) const override;
  std::unique_ptr<std::vector<uint8_t>> EncodeErrorEnvelopeInternal(
      const std::string &error_code, const std::string &error_message,
      const Json::Value *error_details) const override;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_JSON_METHOD_CODEC_H_
