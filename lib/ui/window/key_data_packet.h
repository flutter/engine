// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_KEY_DATA_PACKET_H_
#define FLUTTER_LIB_UI_WINDOW_KEY_DATA_PACKET_H_

#include <string.h>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/key_data.h"


namespace flutter {

class KeyDataPacket {
 public:
  KeyDataPacket(size_t logical_count, size_t total_character_size);
  KeyDataPacket(uint8_t* data, size_t num_bytes);
  ~KeyDataPacket();

  void pushData(const uint8_t* data, size_t data_size);
  inline void pushPhysicalKey(const PhysicalKeyData *event) {
    pushData(reinterpret_cast<const uint8_t*>(event), sizeof(PhysicalKeyData));
  }
  inline void pushLogicalKey(const LogicalKeyData *event) {
    pushData(reinterpret_cast<const uint8_t*>(event), sizeof(LogicalKeyData));
  }
  const std::vector<uint8_t>& data() const { return data_; }

 private:
  std::vector<uint8_t> data_;
  size_t filled_;

  FML_DISALLOW_COPY_AND_ASSIGN(KeyDataPacket);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_H_
