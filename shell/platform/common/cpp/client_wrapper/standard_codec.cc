// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "standard_codec_serializer.h"

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

StandardCodecSerializer::StandardCodecSerializer() = default;

StandardCodecSerializer::~StandardCodecSerializer() = default;

EncodableValue StandardCodecSerializer::ReadValue(
    ByteBufferStreamReader* stream) const {
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
  std::cerr << "Unknown type in StandardCodecSerializer::ReadValue: "
            << static_cast<int>(type) << std::endl;
  return EncodableValue();
}

void StandardCodecSerializer::WriteValue(const EncodableValue& value,
                                         ByteBufferStreamWriter* stream) const {
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

uint32_t StandardCodecSerializer::ReadSize(
    ByteBufferStreamReader* stream) const {
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

void StandardCodecSerializer::WriteSize(uint32_t size,
                                        ByteBufferStreamWriter* stream) const {
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
EncodableValue StandardCodecSerializer::ReadVector(
    ByteBufferStreamReader* stream) const {
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
void StandardCodecSerializer::WriteVector(
    const std::vector<T> vector,
    ByteBufferStreamWriter* stream) const {
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
