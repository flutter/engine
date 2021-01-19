// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_KEY_DATA_MESSAGE_H_
#define FLUTTER_LIB_UI_WINDOW_KEY_DATA_MESSAGE_H_

#include <string.h>
#include <vector>
#include <functional>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/key_data.h"

namespace flutter {

typedef std::function<void (bool)> KeyDataMessageCallback;

class KeyDataMessage {
 public:
  // Build a KeyDataMessage by incrementally fill in data.
  //
  // The `character_data_size` is number of bytes to contain the character data.
  KeyDataMessage(size_t character_data_size);
  ~KeyDataMessage();

  const std::vector<uint8_t>& data() const { return data_; }

  void SetKeyData(const KeyData& event);

  // Set character data to the proper position, which should not be terminated
  // by a null character (length controled by character_data_size).
  void SetCharacter(const char* characters);

 private:
  std::vector<uint8_t>& data() { return data_; }

  size_t CharacterSizeStart_() { return 0; }
  size_t KeyDataStart_() { return CharacterSizeStart_() + sizeof(uint64_t); }
  size_t CharacterStart_() { return KeyDataStart_() + sizeof(KeyData); }

  std::vector<uint8_t> data_;

  FML_DISALLOW_COPY_AND_ASSIGN(KeyDataMessage);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_MESSAGE_H_
