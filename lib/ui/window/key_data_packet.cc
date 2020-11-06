// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_packet.h"
#include "flutter/fml/logging.h"

namespace flutter {

KeyDataPacket::KeyDataPacket(uint8_t* data, size_t num_bytes)
    : data_(data, data + num_bytes) {}

KeyDataPacket::KeyDataPacket(size_t num_bytes)
    : data_(num_bytes) {}

KeyDataPacket::~KeyDataPacket() = default;

KeyDataPacketBuilder::KeyDataPacketBuilder(size_t logical_count, size_t total_character_size)
    : KeyDataPacket(
        logical_count * sizeof(LogicalKeyData) +  // logical data
        sizeof(PhysicalKeyData) +                 // physical data
        total_character_size
      ),
      logical_count_(logical_count) {
}

KeyDataPacketBuilder::~KeyDataPacketBuilder() = default;

void KeyDataPacketBuilder::SetPhysicalData(const PhysicalKeyData& event) {
  memcpy(&data()[PhysicalDataStart_()], &event, sizeof(PhysicalKeyData));
}

void KeyDataPacketBuilder::SetLogicalData(const LogicalKeyData& event, int i) {
  memcpy(&data()[LogicalDataStart_() + i * sizeof(LogicalKeyData)], &event, sizeof(LogicalKeyData));
}

void KeyDataPacketBuilder::SetCharacters(const uint8_t* characters) {
  memcpy(&data()[CharactersStart_()], characters, data().size() - CharactersStart_());
}

}  // namespace flutter
