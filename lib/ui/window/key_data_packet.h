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
  KeyDataPacket(uint8_t* data, size_t num_bytes, size_t logical_count);
  ~KeyDataPacket();

  void SetPhysicalData(const PhysicalKeyData& event);
  void SetLogicalData(const LogicalKeyData& event, int i);
  void SetCharacters(const uint8_t* data);
  const std::vector<uint8_t>& data() const { return data_; }

 private:
  size_t LogicalDataStart_() { return 0; }
  size_t PhysicalDataStart_() { return LogicalDataStart_() + logical_count_ * sizeof(LogicalKeyData); }
  size_t CharactersStart_() { return PhysicalDataStart_() + sizeof(PhysicalKeyData); }

  std::vector<uint8_t> data_;
  size_t logical_count_;

  FML_DISALLOW_COPY_AND_ASSIGN(KeyDataPacket);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_H_
