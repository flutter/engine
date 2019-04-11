// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_MESSAGE_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_MESSAGE_CODEC_H_

#include "encodable_value.h"
#include "message_codec.h"

namespace flutter {

class StandardCodecByteStreamReader;
class StandardCodecByteStreamWriter;

// A binary message encoding/decoding mechanism for communications to/from the
// Flutter engine via message channels.
class StandardMessageCodec : public MessageCodec<EncodableValue> {
 public:
  // Returns the shared instance of the codec.
  static const StandardMessageCodec& GetInstance();

  ~StandardMessageCodec();

  // Prevent copying.
  StandardMessageCodec(StandardMessageCodec const&) = delete;
  StandardMessageCodec& operator=(StandardMessageCodec const&) = delete;

 protected:
  // Instances should be obtained via GetInstance.
  StandardMessageCodec();

  // |flutter::MessageCodec|
  std::unique_ptr<EncodableValue> DecodeMessageInternal(
      const uint8_t* binary_message,
      const size_t message_size) const override;

  // |flutter::MessageCodec|
  std::unique_ptr<std::vector<uint8_t>> EncodeMessageInternal(
      const EncodableValue& message) const override;

 private:
  // Reads and returns then next value from |stream|.
  EncodableValue ReadValue(StandardCodecByteStreamReader* stream) const;

  // Writes the encoding of |value| to |stream|.
  void WriteValue(const EncodableValue& value,
                  StandardCodecByteStreamWriter* stream) const;

  // Reads the variable-length size from the current position in |stream|.
  uint32_t ReadSize(StandardCodecByteStreamReader* stream) const;

  // Writes the variable-length size encoding to |stream|.
  void WriteSize(uint32_t size, StandardCodecByteStreamWriter* stream) const;

  // Reads a fixed-type list whose values are of type T from the current
  // position in |stream|, and returns it as the corresponding EncodableValue.
  // |T| must correspond to one of the support list value types of
  // EncodableValue.
  template <typename T>
  EncodableValue ReadVector(StandardCodecByteStreamReader* stream) const;

  // Writes |vector| to |stream| as a fixed-type list. |T| must correspond to
  // one of the support list value types of EncodableValue.
  template <typename T>
  void WriteVector(const std::vector<T> vector,
                   StandardCodecByteStreamWriter* stream) const;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_MESSAGE_CODEC_H_
