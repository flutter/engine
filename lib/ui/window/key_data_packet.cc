// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data_packet.h"
#include "flutter/fml/logging.h"

namespace flutter {

KeyDataPacket::KeyDataPacket(size_t logical_count, size_t total_character_size)
    : data_(sizeof(PhysicalKeyData) + total_character_size + logical_count * sizeof(LogicalKeyData)), filled_(0) {}

KeyDataPacket::KeyDataPacket(uint8_t* data, size_t num_bytes)
    : data_(data, data + num_bytes), filled_(num_bytes) {}

KeyDataPacket::~KeyDataPacket() = default;

void KeyDataPacket::pushData(const uint8_t* data, size_t data_size) {
  FML_DCHECK(data_size + filled_ <= data_.size());
  memcpy(&data_[filled_], &data, data_size);
  filled_ += data_size;
}

}  // namespace flutter
