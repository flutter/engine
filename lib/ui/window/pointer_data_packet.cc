// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/pointer_data_packet.h"

#include <cstring>

namespace flutter {

PointerDataPacket::PointerDataPacket(size_t count)
    : data_(count * sizeof(PointerData)) {}

PointerDataPacket::PointerDataPacket(uint8_t* data, size_t num_bytes)
    : data_(data, data + num_bytes) {}

PointerDataPacket::~PointerDataPacket() = default;

void PointerDataPacket::SetPointerData(size_t i, const PointerData& data) {
  memcpy(&data_[i * sizeof(PointerData)], &data, sizeof(PointerData));
}

PointerData PointerDataPacket::GetPointerData(size_t i) const {
  PointerData result;
  memcpy(&result, &data_[i * sizeof(PointerData)], sizeof(PointerData));
  return result;
}

size_t PointerDataPacket::Size() const {
  return data_.size() / sizeof(PointerData);
}

}  // namespace flutter
