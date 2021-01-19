// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_message.h"
#include "flutter/fml/logging.h"

namespace flutter {

KeyDataMessage::KeyDataMessage(size_t character_data_size)
    : data_(sizeof(uint64_t) + sizeof(KeyData) + character_data_size) {
  uint64_t size64 = character_data_size;
  memcpy(&data()[CharacterSizeStart_()], &size64, sizeof(size64));
}

KeyDataMessage::~KeyDataMessage() = default;

void KeyDataMessage::SetKeyData(const KeyData& event) {
  memcpy(&data()[KeyDataStart_()], &event, sizeof(KeyData));
}

void KeyDataMessage::SetCharacter(const char* character) {
  if (character != nullptr) {
    memcpy(data().data() + CharacterStart_(), character,
           data().size() - CharacterStart_());
  }
}

}  // namespace flutter
