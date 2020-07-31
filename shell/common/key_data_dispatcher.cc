// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/key_data_dispatcher.h"

namespace flutter {

KeyDataDispatcher::~KeyDataDispatcher() = default;
DefaultKeyDataDispatcher::~DefaultKeyDataDispatcher() = default;

void DefaultKeyDataDispatcher::DispatchKeyPacket(
    std::unique_ptr<KeyDataPacket> packet) {
  delegate_.DoKeyDispatchPacket(std::move(packet));
}

}  // namespace flutter
