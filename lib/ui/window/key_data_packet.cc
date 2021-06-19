// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_packet.h"

#include <cstring>

#include "flutter/fml/logging.h"

namespace flutter {

// Copies `size_in_bytes` bytes of data from `source` into `dest`, and returns
// the position in destination immdiately after.
static uint8_t* copy(uint8_t* dest, uint8_t* source, size_t size_in_bytes) {
  if (size_in_bytes != 0) {
    memcpy(dest, source, size_in_bytes);
  }
  return dest + size_in_bytes;
}

KeyDataPacket::KeyDataPacket(
    size_t num_events,
    const KeyData* events,
    const char* character,
    const uint8_t* raw_event,
    size_t raw_event_size) {
  size_t char_size = character == nullptr ? 0 : strlen(character);
  if (events == nullptr) {
    num_events = 0;
  }
  if (raw_event == nullptr || raw_event_size == 0) {
    return;
  }
  const size_t data_size = sizeof(uint64_t) // num_events
      + num_events * sizeof(KeyData) // events
      + sizeof(uint64_t) // char_size
      + char_size // character
      + sizeof(uint64_t) // raw_event_size
      + raw_event_size; // raw_event
  data_.resize(data_size);

  uint8_t* position = data_.data();

  const uint64_t num_events_64 = num_events;
  const uint64_t char_size_64 = char_size;
  const uint64_t raw_event_size_64 = raw_event_size;
  position = copy(position, &char_size_64, sizeof(char_size_64));
  position = copy(position, character, char_size);
  position = copy(position, &num_events_64, sizeof(num_events_64));
  position = copy(position, events, num_events * sizeof(KeyData));
  position = copy(position, &raw_event_size_64, sizeof(raw_event_size_64));
  position = copy(position, raw_event, sizeof(raw_event_size_64));
  if (position != data_.data() + data_size) {
    data_.resize(0);
  }
}

KeyDataPacket::~KeyDataPacket() = default;

}  // namespace flutter
