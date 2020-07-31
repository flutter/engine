// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/key_data.h"

#include <string.h>

namespace flutter {

static_assert(sizeof(LogicalKeyData) == kBytesPerKeyField * kLogicalKeyDataFieldCount,
              "LogicalKeyData has the wrong size");

void LogicalKeyData::Clear() {
  memset(this, 0, sizeof(LogicalKeyData));
}

static_assert(sizeof(PhysicalKeyData) == kBytesPerKeyField * kKeyDataFieldCount,
              "PhysicalKeyData has the wrong size");

void PhysicalKeyData::Clear() {
  memset(this, 0, sizeof(PhysicalKeyData));
}

}  // namespace flutter
