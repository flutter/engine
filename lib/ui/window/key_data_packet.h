// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_KEY_DATA_MESSAGE_H_
#define FLUTTER_LIB_UI_WINDOW_KEY_DATA_MESSAGE_H_

#include <vector>
#include <functional>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/key_data.h"

namespace flutter {

class KeyDataPacket {
 public:
  // Build a KeyDataPacket by incrementally fill in data.
  //
  // The `character_data_size` is number of bytes to contain the character data.
  KeyDataPacket(size_t character_data_size);
  ~KeyDataPacket();

  const std::vector<uint8_t>& data() const { return data_; }

  void SetKeyData(const KeyData& event);

  // Set character data to the proper position, which should not be terminated
  // by a null character (length controled by character_data_size).
  void SetCharacter(const char* characters);

 private:
  size_t CharacterSizeStart_() { return 0; }
  size_t KeyDataStart_() { return CharacterSizeStart_() + sizeof(uint64_t); }
  size_t CharacterStart_() { return KeyDataStart_() + sizeof(KeyData); }

  std::vector<uint8_t> data_;

  FML_DISALLOW_COPY_AND_ASSIGN(KeyDataPacket);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_MESSAGE_H_
