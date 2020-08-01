// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_KEY_DATA_H_
#define FLUTTER_LIB_UI_WINDOW_KEY_DATA_H_

#include <stdint.h>

namespace flutter {

// If this value changes, update the key data unpacking code in hooks.dart.
static constexpr int kPhysicalKeyDataFieldCount = 3;
static constexpr int kLogicalKeyDataFieldCount = 3;
static constexpr int kBytesPerKeyField = sizeof(int64_t);

// Must match the KeyChange enum in hardware_keyboard.dart.
enum class KeyChange : int64_t {
  kUp = 0,
  kDown,
  kSync,
  kCancel,
};

// This structure is unpacked by hooks.dart.
struct alignas(8) LogicalKeyData {
  int64_t character_size;
  KeyChange change;
  int64_t key;

  void Clear();
};

// This structure is unpacked by hooks.dart.
struct alignas(8) PhysicalKeyData {
  int64_t timestamp;
  KeyChange change;
  int64_t key;

  void Clear();
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_H_
