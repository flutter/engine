// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/client_wrapper/include/flutter/string_message_codec.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {

// Validates round-trip encoding and decoding of |value|, and checks that the
// encoded value matches |expected_encoding|.
static void CheckEncodeDecode(const std::string& value,
                              const std::vector<uint8_t>& expected_encoding) {
  const StringMessageCodec& codec = StringMessageCodec::GetInstance();
  auto encoded = codec.EncodeMessage(value);
  ASSERT_TRUE(encoded);
  EXPECT_EQ(*encoded, expected_encoding);

  auto decoded = codec.DecodeMessage(*encoded);
  EXPECT_EQ(value, *decoded);
}

TEST(StringMessageCodec, CanEncodeAndDecodeString) {
  std::vector<uint8_t> bytes = {0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                                0x77, 0x6f, 0x72, 0x6c, 0x64};
  CheckEncodeDecode(u8"hello world", bytes);
}

TEST(StringMessageCodec, CanEncodeAndDecodeStringWithNonAsciiCodePoint) {
  std::vector<uint8_t> bytes = {0x68, 0xe2, 0x98, 0xba, 0x77};
  CheckEncodeDecode(u8"h\u263Aw", bytes);
}

TEST(StringMessageCodec, CanEncodeAndDecodeStringWithNonBMPCodePoint) {
  std::vector<uint8_t> bytes = {0x68, 0xf0, 0x9f, 0x98, 0x82, 0x77};
  CheckEncodeDecode(u8"h\U0001F602w", bytes);
}

TEST(StringMessageCodec, CanEncodeAndDecodeEmptyString) {
  std::vector<uint8_t> bytes = {};
  CheckEncodeDecode(u8"", bytes);
}

}  // namespace flutter
