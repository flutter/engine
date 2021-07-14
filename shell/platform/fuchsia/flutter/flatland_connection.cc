// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flatland_connection.h"

namespace flutter_runner {

FlatlandConnection::FlatlandConnection(
    std::string debug_label,
    fml::closure error_callback,
    on_frame_presented_event on_frame_presented_callback,
    uint64_t max_frames_in_flight,
    fml::TimeDelta vsync_offset) {}

FlatlandConnection::~FlatlandConnection() = default;

}  // namespace flutter_runner
