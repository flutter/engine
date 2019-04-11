// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_message_codec.h"

#include <cmath>
#include <map>
#include <vector>

#include "gtest/gtest.h"

namespace flutter {

// Returns true if |a| and |b| have equivalent values, recursively comparing
// the contents of collections (unlike the < operator defined on EncodableValue,
// which doesn't consider different collections with the same contents to be
// the same).
static bool ValuesAreEqual(const EncodableValue& a, const EncodableValue& b) {
  if (a.type() != b.type()) {
    return false;
  }

  switch (a.type()) {
    case EncodableValue::Type::kNull:
      return true;
    case EncodableValue::Type::kBool:
      return a.BoolValue() == b.BoolValue();
    case EncodableValue::Type::kInt:
      return a.IntValue() == b.IntValue();
    case EncodableValue::Type::kLong:
      return a.LongValue() == b.LongValue();
    case EncodableValue::Type::kDouble:
      // This is a crude epsilon, but fine for the values in the the unit tests.
      return std::abs(a.DoubleValue() - b.DoubleValue()) < 0.0001l;
    case EncodableValue::Type::kString:
      return a.StringValue() == b.StringValue();
    case EncodableValue::Type::kByteList:
      return a.ByteListValue() == b.ByteListValue();
    case EncodableValue::Type::kIntList:
      return a.IntListValue() == b.IntListValue();
    case EncodableValue::Type::kLongList:
      return a.LongListValue() == b.LongListValue();
    case EncodableValue::Type::kDoubleList:
      return a.DoubleListValue() == b.DoubleListValue();
    case EncodableValue::Type::kList: {
      const auto& a_list = a.ListValue();
      const auto& b_list = b.ListValue();
      if (a_list.size() != b_list.size()) {
        return false;
      }
      for (size_t i = 0; i < a_list.size(); ++i) {
        if (!ValuesAreEqual(a_list[0], b_list[0])) {
          return false;
        }
      }
      return true;
    }
    case EncodableValue::Type::kMap: {
      const auto& a_map = a.MapValue();
      const auto& b_map = b.MapValue();
      if (a_map.size() != b_map.size()) {
        return false;
      }
      // Store references to all the keys in |b|.
      std::vector<const EncodableValue*> unmatched_b_keys;
      for (auto& pair : b_map) {
        unmatched_b_keys.push_back(&pair.first);
      }
      // For each key,value in |a|, see if any of the not-yet-matched key,value
      // pairs in |b| match by value; if so, remove that match and continue.
      for (const auto& pair : a_map) {
        bool found_match = false;
        for (size_t i = 0; i < unmatched_b_keys.size(); ++i) {
          const EncodableValue& b_key = *unmatched_b_keys[i];
          if (ValuesAreEqual(pair.first, b_key) &&
              ValuesAreEqual(pair.second, b_map.at(b_key))) {
            found_match = true;
            unmatched_b_keys.erase(unmatched_b_keys.begin() + i);
            break;
          }
        }
        if (!found_match) {
          return false;
        }
      }
      // If all entries had matches, consider the maps equal.
      return true;
    }
  }
  assert(false);
  return false;
}

// Validates round-trip encoding and decoding of |value|, and checks that the
// encoded value matches |expected_encoding|.
static void CheckEncodeDecode(const EncodableValue& value,
                              const std::vector<uint8_t>& expected_encoding) {
  const StandardMessageCodec& codec = StandardMessageCodec::GetInstance();
  auto encoded = codec.EncodeMessage(value);
  ASSERT_TRUE(encoded);
  EXPECT_EQ(*encoded, expected_encoding);

  auto decoded = codec.DecodeMessage(*encoded);
  EXPECT_TRUE(ValuesAreEqual(value, *decoded));
}

// Validates round-trip encoding and decoding of |value|, and checks that the
// encoded value has the given prefix and length.
//
// This should be used only for Map, where asserting the order of the elements
// in a test is undesirable.
static void CheckEncodeDecodeWithEncodePrefix(
    const EncodableValue& value,
    const std::vector<uint8_t>& expected_encoding_prefix,
    size_t expected_encoding_length) {
  EXPECT_TRUE(value.IsMap());
  const StandardMessageCodec& codec = StandardMessageCodec::GetInstance();
  auto encoded = codec.EncodeMessage(value);
  ASSERT_TRUE(encoded);

  EXPECT_EQ(encoded->size(), expected_encoding_length);
  ASSERT_GT(encoded->size(), expected_encoding_prefix.size());
  EXPECT_TRUE(std::equal(
      encoded->begin(), encoded->begin() + expected_encoding_prefix.size(),
      expected_encoding_prefix.begin(), expected_encoding_prefix.end()));

  auto decoded = codec.DecodeMessage(*encoded);
  EXPECT_TRUE(ValuesAreEqual(value, *decoded));
}

TEST(StandardMessageCodec, CanEncodeAndDecodeNull) {
  std::vector<uint8_t> bytes = {0x00};
  CheckEncodeDecode(EncodableValue(), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeTrue) {
  std::vector<uint8_t> bytes = {0x01};
  CheckEncodeDecode(EncodableValue(true), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeFalse) {
  std::vector<uint8_t> bytes = {0x02};
  CheckEncodeDecode(EncodableValue(false), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeInt32) {
  std::vector<uint8_t> bytes = {0x03, 0x78, 0x56, 0x34, 0x12};
  CheckEncodeDecode(EncodableValue(0x12345678), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeInt64) {
  std::vector<uint8_t> bytes = {0x04, 0xef, 0xcd, 0xab, 0x90,
                                0x78, 0x56, 0x34, 0x12};
  CheckEncodeDecode(EncodableValue(0x1234567890abcdef), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeDouble) {
  std::vector<uint8_t> bytes = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40};
  CheckEncodeDecode(EncodableValue(3.14159265358979311599796346854), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeString) {
  std::vector<uint8_t> bytes = {0x07, 0x0b, 0x68, 0x65, 0x6c, 0x6c, 0x6f,
                                0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64};
  CheckEncodeDecode(EncodableValue(u8"hello world"), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeStringWithNonAsciiCodePoint) {
  std::vector<uint8_t> bytes = {0x07, 0x05, 0x68, 0xe2, 0x98, 0xba, 0x77};
  CheckEncodeDecode(EncodableValue(u8"h\u263Aw"), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeStringWithNonBMPCodePoint) {
  std::vector<uint8_t> bytes = {0x07, 0x06, 0x68, 0xf0, 0x9f, 0x98, 0x82, 0x77};
  CheckEncodeDecode(EncodableValue(u8"h\U0001F602w"), bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeList) {
  std::vector<uint8_t> bytes = {
      0x0c, 0x05, 0x00, 0x07, 0x05, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x06,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x85, 0xeb, 0x51, 0xb8, 0x1e,
      0x09, 0x40, 0x03, 0x2f, 0x00, 0x00, 0x00, 0x0c, 0x02, 0x03, 0x2a,
      0x00, 0x00, 0x00, 0x07, 0x06, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64,
  };
  EncodableValue value(EncodableList{
      EncodableValue(),
      EncodableValue("hello"),
      EncodableValue(3.14),
      EncodableValue(47),
      EncodableValue(EncodableList{
          EncodableValue(42),
          EncodableValue("nested"),
      }),
  });
  CheckEncodeDecode(value, bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeMap) {
  std::vector<uint8_t> bytes_prefix = {0x0d, 0x04};
  EncodableValue value(EncodableMap{
      {EncodableValue("a"), EncodableValue(3.14)},
      {EncodableValue("b"), EncodableValue(47)},
      {EncodableValue(), EncodableValue()},
      {EncodableValue(3.14), EncodableValue(EncodableList{
                                 EncodableValue("nested"),
                             })},
  });
  CheckEncodeDecodeWithEncodePrefix(value, bytes_prefix, 48);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeByteArray) {
  std::vector<uint8_t> bytes = {0x08, 0x04, 0xba, 0x5e, 0xba, 0x11};
  EncodableValue value(std::vector<uint8_t>{0xba, 0x5e, 0xba, 0x11});
  CheckEncodeDecode(value, bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeInt32Array) {
  std::vector<uint8_t> bytes = {0x09, 0x03, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12,
                                0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
  EncodableValue value(std::vector<int32_t>{0x12345678, -1, 0});
  CheckEncodeDecode(value, bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeInt64Array) {
  std::vector<uint8_t> bytes = {0x0a, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0xef, 0xcd, 0xab, 0x90, 0x78, 0x56, 0x34, 0x12,
                                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  EncodableValue value(std::vector<int64_t>{0x1234567890abcdef, -1});
  CheckEncodeDecode(value, bytes);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeFloat64Array) {
  std::vector<uint8_t> bytes = {0x0b, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x8f, 0x40};
  EncodableValue value(
      std::vector<double>{3.14159265358979311599796346854, 1000.0});
  CheckEncodeDecode(value, bytes);
}

}  // namespace flutter
