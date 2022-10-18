// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pointer_data_packet_merged_task_poster.h"
#include "flutter/fml/make_copyable.h"

#include <cstring>

namespace flutter {

void PointerDataPacketMergedTaskPoster::Dispatch(
    std::unique_ptr<PointerDataPacket> packet,
    const TaskRunners& task_runners,
    const PointerDataPacketMergedTaskPoster::Callback& callback) {
  task_runners.GetUITaskRunner()->PostTask(
      fml::MakeCopyable([packet = std::move(packet), callback]() mutable {
        callback(std::move(packet));
      }));
}

}  // namespace flutter
