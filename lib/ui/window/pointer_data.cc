// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/pointer_data.h"

#include <cstring>

namespace flutter {

// If kPointerDataFieldCount changes, update the corresponding values in:
//
//  * platform_dispatcher.dart
//  * AndroidTouchProcessor.java
static constexpr int kPointerDataFieldCount = 36;
static constexpr int kBytesPerField = sizeof(int64_t);

static_assert(sizeof(PointerData) == kBytesPerField * kPointerDataFieldCount,
              "PointerData has the wrong size");

void PointerData::Clear() {
  memset(this, 0, sizeof(PointerData));
}

}  // namespace flutter
