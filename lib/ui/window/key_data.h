// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_KEY_DATA_H_
#define FLUTTER_LIB_UI_WINDOW_KEY_DATA_H_

#include <stdint.h>

namespace flutter {

// If this value changes, update the key data unpacking code in hooks.dart.
static constexpr int kKeyDataFieldCount = 6;
static constexpr int kBytesPerKeyField = sizeof(int64_t);

// Must match the KeyChange enum in ui/key.dart.
enum class KeyChange : int64_t {
  kDown = 0,
  kUp,
  kRepeat,
};

// This structure is unpacked by hooks.dart.
struct alignas(8) KeyData {
  // Timestamp in microseconds from an arbitrary and consistant start point
  uint64_t timestamp;
  KeyChange change;
  uint64_t physical;
  uint64_t logical;
  uint64_t locks;
  uint64_t synthesized;

  void Clear();
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_H_
