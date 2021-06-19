// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_KEY_DATA_MESSAGE_H_
#define FLUTTER_LIB_UI_WINDOW_KEY_DATA_MESSAGE_H_

#include <functional>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/key_data.h"

namespace flutter {

// A byte stream representing a key message, to be sent to the framework.
class KeyDataPacket {
 public:
  // Build the key data packet by providing information.
  //
  // The `events` is a C-array of KeyData of length `num_events`, which can be
  // nullptr and 0 respectively.
  //
  // The `character` is a nullable C-string that ends with a '\0'.
  //
  // The `raw_event` is a byte stream of length `raw_event`, which must not be
  // nullptr and 0 respectively.
  //
  // The created packet may not be valid. Check validity with `valid`.  Failing
  // the validity check indicates a kInternalInconsistency error.
  KeyDataPacket(
    size_t num_events,
    const KeyData* events,
    const char* character,
    const uint8_t* raw_event,
    size_t raw_event_size);
  ~KeyDataPacket();

  // Prevent copying.
  KeyDataPacket(KeyDataPacket const&) = delete;
  KeyDataPacket& operator=(KeyDataPacket const&) = delete;

  const std::vector<uint8_t>& data() const { return data_; }

  bool valid() const { return data_.size() != 0; }

 private:
  // Packet structure:
  // |  NumEvents   |
  // |    Events    |
  // | CharDataSize |
  // |   CharData   |
  // | RawEventSize |
  // |   RawEvent   |

  std::vector<uint8_t> data_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_MESSAGE_H_
