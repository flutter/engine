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

// A byte stream representing a key event, to be sent to the framework.
class KeyDataPacket {
 public:
  // Build the key data packet by providing information. 
  //
  // The `character` is a nullable C-string that ends with a '\0'.
  KeyDataPacket(const KeyData& event, const char* character);
  ~KeyDataPacket();

  const std::vector<uint8_t>& data() const { return data_; }

 private:
  // Packet structure:
  // | CharDataSize |     (1 field)
  // |   Key Data   |     (kKeyDataFieldCount fields)
  // |   CharData   |     (CharDataSize bits)

  size_t CharacterSizeStart_() { return 0; }
  size_t KeyDataStart_() { return CharacterSizeStart_() + sizeof(uint64_t); }
  size_t CharacterStart_() { return KeyDataStart_() + sizeof(KeyData); }

  std::vector<uint8_t> data_;

  FML_DISALLOW_COPY_AND_ASSIGN(KeyDataPacket);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_MESSAGE_H_
