// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_packet.h"

#include <cstring>

#include "flutter/fml/logging.h"

namespace flutter {

KeyDataPacket::KeyDataPacket(const KeyData& event, const char* character) {
  uint64_t char_size = character == nullptr ? 0 : strlen(character);
  data_.resize(sizeof(uint64_t) + sizeof(KeyData) + char_size);
  memcpy(&data_[CharacterSizeStart_()], &char_size, sizeof(char_size));
  memcpy(&data_[KeyDataStart_()], &event, sizeof(KeyData));
  if (character != nullptr) {
    memcpy(data_.data() + CharacterStart_(), character,
           data_.size() - CharacterStart_());
  }
}

KeyDataPacket::~KeyDataPacket() = default;

}  // namespace flutter
