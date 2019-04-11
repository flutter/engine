// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/standard_message_codec.h"

#include <assert.h>
#include <cstring>
#include <iostream>
#include <string>

namespace flutter {

namespace {

// The order/values here must match the constants in message_codecs.dart.
enum class EncodedType {
  kNull = 0,
  kTrue,
  kFalse,
  kInt32,
  kInt64,
  kLargeInt,  // No longer used. If encountered, treat as kString.
  kFloat64,
  kString,
  kUInt8List,
  kInt32List,
  kInt64List,
  kFloat64List,
  kList,
  kMap,
};

// Returns the encoded type that should be written when serializing |value|.
EncodedType EncodedTypeForValue(const EncodableValue& value) {
  switch (value.type()) {
    case EncodableValue::Type::kNull:
      return EncodedType::kNull;
    case EncodableValue::Type::kBool:
      return value.BoolValue() ? EncodedType::kTrue : EncodedType::kFalse;
    case EncodableValue::Type::kInt:
      return EncodedType::kInt32;
    case EncodableValue::Type::kLong:
      return EncodedType::kInt64;
    case EncodableValue::Type::kDouble:
      return EncodedType::kFloat64;
    case EncodableValue::Type::kString:
      return EncodedType::kString;
    case EncodableValue::Type::kByteList:
      return EncodedType::kUInt8List;
    case EncodableValue::Type::kIntList:
      return EncodedType::kInt32List;
    case EncodableValue::Type::kLongList:
      return EncodedType::kInt64List;
    case EncodableValue::Type::kDoubleList:
      return EncodedType::kFloat64List;
    case EncodableValue::Type::kList:
      return EncodedType::kList;
    case EncodableValue::Type::kMap:
      return EncodedType::kMap;
  }
  assert(false);
  return EncodedType::kNull;
}

}  // namespace

// Wraps an array of bytes with utility methods for treating it as a readable
// stream.
class StandardCodecByteStreamReader {
 public:
  // Createa a reader reading from |bytes|, which must have a length of |size|.
  // |bytes| must remain valid for the lifetime of this object.
  explicit StandardCodecByteStreamReader(const uint8_t* bytes, size_t size)
      : bytes_(bytes), size_(size) {}

  // Reads and returns the next byte from the stream.
  uint8_t ReadByte() {
    if (location_ >= size_) {
      std::cerr << "Invalid read in StandardCodecByteStreamReader" << std::endl;
      return 0;
    }
    return bytes_[location_++];
  }

  // Reads the next |length| bytes from the stream into |buffer|. The caller
  // is responsible for ensuring that |buffer| is large enough.
  void ReadBytes(uint8_t* buffer, size_t length) {
    if (location_ + length > size_) {
      std::cerr << "Invalid read in StandardCodecByteStreamReader" << std::endl;
      return;
    }
    std::memcpy(buffer, &bytes_[location_], length);
    location_ += length;
  }

  // Advances the read cursor to the next multiple of |alignment| relative to
  // the start of the wrapped byte buffer, unless it is already aligned.
  void ReadAlignment(uint8_t alignment) {
    uint8_t mod = location_ % alignment;
    if (mod) {
      location_ += alignment - mod;
    }
  }

 private:
  // The buffer to read from.
  const uint8_t* bytes_;
  // The total size of the buffer.
  size_t size_;
  // The current read location.
  size_t location_ = 0;
};

// Wraps an array of bytes with utility methods for treating it as a writable
// stream.
class StandardCodecByteStreamWriter {
 public:
  // Createa a writter that writes into |buffer|.
  // |buffer| must remain valid for the lifetime of this object.
  explicit StandardCodecByteStreamWriter(std::vector<uint8_t>* buffer)
      : bytes_(buffer) {
    assert(buffer);
  }

  // Writes |byte| to the wrapped buffer.
  void WriteByte(uint8_t byte) { bytes_->push_back(byte); }

  // Writes the next |length| bytes from |bytes| into the wrapped buffer.
  // The caller is responsible for ensuring that |buffer| is large enough.
  void WriteBytes(const uint8_t* bytes, size_t length) {
    assert(length > 0);
    bytes_->insert(bytes_->end(), bytes, bytes + length);
  }

  // Writes 0s until the next multiple of |alignment| relative to
  // the start of the wrapped byte buffer, unless the write positition is
  // already aligned.
  void WriteAlignment(uint8_t alignment) {
    uint8_t mod = bytes_->size() % alignment;
    if (mod) {
      for (int i = 0; i < alignment - mod; ++i) {
        WriteByte(0);
      }
    }
  }

 private:
  // The buffer to write to.
  std::vector<uint8_t>* bytes_;
};

// static
const StandardMessageCodec& StandardMessageCodec::GetInstance() {
  static StandardMessageCodec sInstance;
  return sInstance;
}

StandardMessageCodec::StandardMessageCodec() = default;

StandardMessageCodec::~StandardMessageCodec() = default;

std::unique_ptr<std::vector<uint8_t>>
StandardMessageCodec::EncodeMessageInternal(
    const EncodableValue& message) const {
  auto encoded = std::make_unique<std::vector<uint8_t>>();
  StandardCodecByteStreamWriter stream(encoded.get());
  WriteValue(message, &stream);
  return encoded;
}

std::unique_ptr<EncodableValue> StandardMessageCodec::DecodeMessageInternal(
    const uint8_t* binary_message,
    const size_t message_size) const {
  StandardCodecByteStreamReader stream(binary_message, message_size);
  return std::make_unique<EncodableValue>(ReadValue(&stream));
}

EncodableValue StandardMessageCodec::ReadValue(
    StandardCodecByteStreamReader* stream) const {
  EncodedType type = static_cast<EncodedType>(stream->ReadByte());
  ;
  switch (type) {
    case EncodedType::kNull:
      return EncodableValue();
    case EncodedType::kTrue:
      return EncodableValue(true);
    case EncodedType::kFalse:
      return EncodableValue(false);
    case EncodedType::kInt32: {
      int32_t int_value = 0;
      stream->ReadBytes(reinterpret_cast<uint8_t*>(&int_value), 4);
      return EncodableValue(int_value);
    }
    case EncodedType::kInt64: {
      int64_t long_value = 0;
      stream->ReadBytes(reinterpret_cast<uint8_t*>(&long_value), 8);
      return EncodableValue(long_value);
    }
    case EncodedType::kFloat64: {
      double double_value = 0;
      stream->ReadAlignment(8);
      stream->ReadBytes(reinterpret_cast<uint8_t*>(&double_value), 8);
      return EncodableValue(double_value);
    }
    case EncodedType::kLargeInt:
    case EncodedType::kString: {
      int32_t size = ReadSize(stream);
      std::string string_value;
      string_value.resize(size);
      stream->ReadBytes(reinterpret_cast<uint8_t*>(&string_value[0]), size);
      return EncodableValue(string_value);
    }
    case EncodedType::kUInt8List:
      return ReadVector<uint8_t>(stream);
    case EncodedType::kInt32List:
      return ReadVector<int32_t>(stream);
    case EncodedType::kInt64List:
      return ReadVector<int64_t>(stream);
    case EncodedType::kFloat64List:
      return ReadVector<double>(stream);
    case EncodedType::kList: {
      int32_t length = ReadSize(stream);
      EncodableList list_value;
      list_value.reserve(length);
      for (int32_t i = 0; i < length; ++i) {
        list_value.push_back(ReadValue(stream));
      }
      return EncodableValue(list_value);
    }
    case EncodedType::kMap: {
      int32_t length = ReadSize(stream);
      EncodableMap map_value;
      for (int32_t i = 0; i < length; ++i) {
        EncodableValue key = ReadValue(stream);
        EncodableValue value = ReadValue(stream);
        map_value.emplace(std::move(key), std::move(value));
      }
      return EncodableValue(map_value);
    }
  }
  std::cerr << "Unknown type in StandardMessageCodec::ReadValue: "
            << static_cast<int>(type) << std::endl;
  return EncodableValue();
}

void StandardMessageCodec::WriteValue(
    const EncodableValue& value,
    StandardCodecByteStreamWriter* stream) const {
  stream->WriteByte(static_cast<uint8_t>(EncodedTypeForValue(value)));
  switch (value.type()) {
    case EncodableValue::Type::kNull:
    case EncodableValue::Type::kBool:
      // Null and bool are encoded directly in the type.
      break;
    case EncodableValue::Type::kInt: {
      int32_t int_value = value.IntValue();
      stream->WriteBytes(reinterpret_cast<const uint8_t*>(&int_value), 4);
      break;
    }
    case EncodableValue::Type::kLong: {
      int64_t long_value = value.LongValue();
      stream->WriteBytes(reinterpret_cast<const uint8_t*>(&long_value), 8);
      break;
    }
    case EncodableValue::Type::kDouble: {
      stream->WriteAlignment(8);
      double double_value = value.DoubleValue();
      stream->WriteBytes(reinterpret_cast<const uint8_t*>(&double_value), 8);
      break;
    }
    case EncodableValue::Type::kString: {
      const auto& string_value = value.StringValue();
      size_t size = string_value.size();
      WriteSize(size, stream);
      stream->WriteBytes(reinterpret_cast<const uint8_t*>(string_value.data()),
                         size);
      break;
    }
    case EncodableValue::Type::kByteList:
      WriteVector(value.ByteListValue(), stream);
      break;
    case EncodableValue::Type::kIntList:
      WriteVector(value.IntListValue(), stream);
      break;
    case EncodableValue::Type::kLongList:
      WriteVector(value.LongListValue(), stream);
      break;
    case EncodableValue::Type::kDoubleList:
      WriteVector(value.DoubleListValue(), stream);
      break;
    case EncodableValue::Type::kList:
      WriteSize(value.ListValue().size(), stream);
      for (const auto& item : value.ListValue()) {
        WriteValue(item, stream);
      }
      break;
    case EncodableValue::Type::kMap:
      WriteSize(value.MapValue().size(), stream);
      for (const auto& pair : value.MapValue()) {
        WriteValue(pair.first, stream);
        WriteValue(pair.second, stream);
      }
      break;
  }
}

uint32_t StandardMessageCodec::ReadSize(
    StandardCodecByteStreamReader* stream) const {
  uint8_t byte = stream->ReadByte();
  if (byte < 254) {
    return byte;
  } else if (byte == 254) {
    uint16_t value;
    stream->ReadBytes(reinterpret_cast<uint8_t*>(&value), 2);
    return value;
  } else {
    uint32_t value;
    stream->ReadBytes(reinterpret_cast<uint8_t*>(&value), 4);
    return value;
  }
}

void StandardMessageCodec::WriteSize(
    uint32_t size,
    StandardCodecByteStreamWriter* stream) const {
  if (size < 254) {
    stream->WriteByte(static_cast<uint8_t>(size));
  } else if (size <= 0xffff) {
    stream->WriteByte(254);
    uint16_t value = static_cast<uint16_t>(size);
    stream->WriteBytes(reinterpret_cast<uint8_t*>(&value), 2);
  } else {
    stream->WriteByte(255);
    stream->WriteBytes(reinterpret_cast<uint8_t*>(&size), 4);
  }
}

template <typename T>
EncodableValue StandardMessageCodec::ReadVector(
    StandardCodecByteStreamReader* stream) const {
  int32_t count = ReadSize(stream);
  std::vector<T> vector;
  vector.resize(count);
  size_t type_size = sizeof(T);
  if (type_size > 1) {
    stream->ReadAlignment(type_size);
  }
  stream->ReadBytes(reinterpret_cast<uint8_t*>(vector.data()),
                    count * type_size);
  return EncodableValue(vector);
}

template <typename T>
void StandardMessageCodec::WriteVector(
    const std::vector<T> vector,
    StandardCodecByteStreamWriter* stream) const {
  size_t count = vector.size();
  WriteSize(count, stream);
  size_t type_size = sizeof(T);
  if (type_size > 1) {
    stream->WriteAlignment(type_size);
  }
  stream->WriteBytes(reinterpret_cast<const uint8_t*>(vector.data()),
                     count * type_size);
}

}  // namespace flutter
