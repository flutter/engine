// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STRING_MESSAGE_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STRING_MESSAGE_CODEC_H_

#include <memory>
#include <string>

#include "message_codec.h"

namespace flutter {

// An implementation of MethodCodec that uses UTF-8 strings as the
// serialization.
class StringMessageCodec : public MessageCodec<std::string> {
 public:
  // Returns an instance of the codec.
  static const StringMessageCodec& GetInstance();

  ~StringMessageCodec() = default;

  // Prevent copying.
  StringMessageCodec(StringMessageCodec const&) = delete;
  StringMessageCodec& operator=(StringMessageCodec const&) = delete;

 protected:
  // Instances should be obtained via GetInstance.
  StringMessageCodec() = default;

  // |flutter::MessageCodec|
  std::unique_ptr<std::string> DecodeMessageInternal(
      const uint8_t* binary_message,
      const size_t message_size) const override;

  // |flutter::MessageCodec|
  std::unique_ptr<std::vector<uint8_t>> EncodeMessageInternal(
      const std::string& message) const override;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STRING_MESSAGE_CODEC_H_
