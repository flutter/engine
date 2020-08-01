// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_packet.h"
#include "flutter/fml/logging.h"

namespace flutter {

KeyDataPacket::KeyDataPacket(size_t logical_count, size_t total_character_size)
    : data_(
        logical_count * sizeof(LogicalKeyData) +  // logical data
        sizeof(PhysicalKeyData) +                 // physical data
        total_character_size
      ),
      logical_count_(logical_count) {
}

KeyDataPacket::KeyDataPacket(uint8_t* data, size_t num_bytes, size_t logical_count)
    : data_(data, data + num_bytes), logical_count_(logical_count) {}

KeyDataPacket::~KeyDataPacket() = default;

void KeyDataPacket::SetPhysicalData(const PhysicalKeyData& event) {
  memcpy(&data_[PhysicalDataStart_()], &event, sizeof(PhysicalKeyData));
}

void KeyDataPacket::SetLogicalData(const LogicalKeyData& event, int i) {
  memcpy(&data_[LogicalDataStart_() + i * sizeof(LogicalKeyData)], &event, sizeof(LogicalKeyData));
}

void KeyDataPacket::SetCharacters(const uint8_t* data) {
  memcpy(&data_[CharactersStart_()], data, data_.size() - CharactersStart_());
}

}  // namespace flutter
