// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <vector>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_message_codec.h"
#include "gtest/gtest.h"

namespace flutter {

// Validates round-trip encoding and decoding of |value|, and checks that the
// encoded value matches |expected_encoding|.
//
// If testing with CustomEncodableValues, |serializer| must be provided to
// handle the encoding/decoding, and |custom_comparator| must be provided to
// validate equality since CustomEncodableValue doesn't define a useful ==.
static void CheckEncodeDecode(
    const EncodableValue& value,
    const std::vector<uint8_t>& expected_encoding,
    const StandardCodecSerializer* serializer = nullptr,
    std::function<bool(const EncodableValue& a, const EncodableValue& b)>
        custom_comparator = nullptr) {
  const StandardMessageCodec& codec =
      StandardMessageCodec::GetInstance(serializer);
  auto encoded = codec.EncodeMessage(value);
  ASSERT_TRUE(encoded);
  EXPECT_EQ(*encoded, expected_encoding);

  auto decoded = codec.DecodeMessage(*encoded);
  if (custom_comparator) {
    EXPECT_TRUE(custom_comparator(value, *decoded));
  } else {
    EXPECT_EQ(value, *decoded);
  }
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
  EXPECT_TRUE(std::holds_alternative<EncodableMap>(value));
  const StandardMessageCodec& codec = StandardMessageCodec::GetInstance();
  auto encoded = codec.EncodeMessage(value);
  ASSERT_TRUE(encoded);

  EXPECT_EQ(encoded->size(), expected_encoding_length);
  ASSERT_GT(encoded->size(), expected_encoding_prefix.size());
  EXPECT_TRUE(std::equal(
      encoded->begin(), encoded->begin() + expected_encoding_prefix.size(),
      expected_encoding_prefix.begin(), expected_encoding_prefix.end()));

  auto decoded = codec.DecodeMessage(*encoded);
  EXPECT_EQ(value, *decoded);
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
  CheckEncodeDecode(EncodableValue(INT64_C(0x1234567890abcdef)), bytes);
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

TEST(StandardMessageCodec, CanEncodeAndDecodeEmptyString) {
  std::vector<uint8_t> bytes = {0x07, 0x00};
  CheckEncodeDecode(EncodableValue(u8""), bytes);
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

TEST(StandardMessageCodec, CanEncodeAndDecodeEmptyList) {
  std::vector<uint8_t> bytes = {0x0c, 0x00};
  CheckEncodeDecode(EncodableValue(EncodableList{}), bytes);
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

namespace {

// A representation of a point, for custom type testing.
class Point {
 public:
  Point(int x, int y) : x_(x), y_(y) {}
  ~Point() = default;

  int x() const { return x_; }
  int y() const { return y_; }

  bool operator==(const Point& other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

 private:
  int x_;
  int y_;
};

// A typed binary data object with extra fields, for custom type testing.
class SomeData {
 public:
  SomeData(const std::string label, const std::vector<uint8_t>& data)
      : label_(label), data_(data) {}
  ~SomeData() = default;

  const std::string& label() const { return label_; }
  const std::vector<uint8_t>& data() const { return data_; }

 private:
  std::string label_;
  std::vector<uint8_t> data_;
};

// Codec extension for EncodablePoint.
class TestCodecSerializer : public StandardCodecSerializer {
 public:
  TestCodecSerializer() = default;
  virtual ~TestCodecSerializer() = default;

  static const TestCodecSerializer& GetInstance() {
    static TestCodecSerializer sInstance;
    return sInstance;
  }

  // |TestCodecSerializer|
  EncodableValue ReadValueOfType(uint8_t type,
                                 ByteStreamReader* stream) const override {
    if (type == kPointType) {
      int32_t x = stream->ReadInt32();
      int32_t y = stream->ReadInt32();
      return CustomEncodableValue(Point(x, y));
    } else if (type == kSomeDataType) {
      size_t size = ReadSize(stream);
      std::vector<uint8_t> data;
      data.resize(size);
      stream->ReadBytes(data.data(), size);
      EncodableValue label = ReadValue(stream);
      return CustomEncodableValue(SomeData(std::get<std::string>(label), data));
    }
    return StandardCodecSerializer::ReadValueOfType(type, stream);
  }

  // |TestCodecSerializer|
  void WriteValue(const EncodableValue& value,
                  ByteStreamWriter* stream) const override {
    auto custom_value = std::get_if<CustomEncodableValue>(&value);
    if (!custom_value) {
      StandardCodecSerializer::WriteValue(value, stream);
      return;
    }
    if (custom_value->type() == typeid(Point)) {
      stream->WriteByte(kPointType);
      const Point& point = std::any_cast<Point>(*custom_value);
      stream->WriteInt32(point.x());
      stream->WriteInt32(point.y());
    } else if (custom_value->type() == typeid(SomeData)) {
      stream->WriteByte(kSomeDataType);
      const SomeData& some_data = std::any_cast<SomeData>(*custom_value);
      size_t data_size = some_data.data().size();
      WriteSize(data_size, stream);
      stream->WriteBytes(some_data.data().data(), data_size);
      WriteValue(EncodableValue(some_data.label()), stream);
    }
  }

 private:
  static constexpr uint8_t kPointType = 128;
  static constexpr uint8_t kSomeDataType = 129;
};

}  // namespace

TEST(StandardMessageCodec, CanEncodeAndDecodeSimpleCustomType) {
  std::vector<uint8_t> bytes = {0x80, 0x09, 0x00, 0x00, 0x00,
                                0x10, 0x00, 0x00, 0x00};
  auto point_comparator = [](const EncodableValue& a, const EncodableValue& b) {
    const Point& a_point =
        std::any_cast<Point>(std::get<CustomEncodableValue>(a));
    const Point& b_point =
        std::any_cast<Point>(std::get<CustomEncodableValue>(b));
    return a_point == b_point;
  };
  CheckEncodeDecode(CustomEncodableValue(Point(9, 16)), bytes,
                    &TestCodecSerializer::GetInstance(), point_comparator);
}

TEST(StandardMessageCodec, CanEncodeAndDecodeVariableLengthCustomType) {
  std::vector<uint8_t> bytes = {
      0x81,                                      // custom type
      0x06, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,  // data
      0x07, 0x04,                                // string type and length
      0x74, 0x65, 0x73, 0x74                     // string characters
  };
  auto some_data_comparator = [](const EncodableValue& a,
                                 const EncodableValue& b) {
    const SomeData& data_a =
        std::any_cast<SomeData>(std::get<CustomEncodableValue>(a));
    const SomeData& data_b =
        std::any_cast<SomeData>(std::get<CustomEncodableValue>(b));
    return data_a.data() == data_b.data() && data_a.label() == data_b.label();
  };
  CheckEncodeDecode(CustomEncodableValue(
                        SomeData("test", {0x00, 0x01, 0x02, 0x03, 0x04, 0x05})),
                    bytes, &TestCodecSerializer::GetInstance(),
                    some_data_comparator);
}

}  // namespace flutter
