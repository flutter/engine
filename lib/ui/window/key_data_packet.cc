// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_packet.h"
#include "flutter/fml/logging.h"

namespace flutter {

KeyDataPacket::KeyDataPacket(uint8_t* data, size_t num_bytes)
    : data_(data, data + num_bytes) {}

KeyDataPacket::KeyDataPacket(size_t num_bytes) : data_(num_bytes) {}

KeyDataPacket::~KeyDataPacket() = default;

KeyDataPacketBuilder::KeyDataPacketBuilder(size_t character_data_size)
    : KeyDataPacket(sizeof(uint64_t) + sizeof(KeyData) + character_data_size) {
  uint64_t size64 = character_data_size;
  memcpy(&data()[CharacterSizeStart_()], &size64, sizeof(size64));
}

KeyDataPacketBuilder::~KeyDataPacketBuilder() = default;

void KeyDataPacketBuilder::SetKeyData(const KeyData& event) {
  memcpy(&data()[KeyDataStart_()], &event, sizeof(KeyData));
}

void KeyDataPacketBuilder::SetCharacter(const char* character) {
  if (character != nullptr) {
    memcpy(data().data() + CharacterStart_(), character,
           data().size() - CharacterStart_());
  }
}

}  // namespace flutter
