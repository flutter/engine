// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pointer_data_packet_merged_task_poster.h"

#include <cstring>

namespace flutter {

void PointerDataPacketMergedTaskPoster::Dispatch(
    std::unique_ptr<PointerDataPacket> packet,
    uint64_t flow_id,
    const TaskRunners& task_runners,
    const fml::WeakPtr<Engine>& weak_engine) {
  task_runners_.GetUITaskRunner()->PostTask(
      fml::MakeCopyable([engine = weak_engine_, packet = std::move(packet),
                         flow_id = next_pointer_flow_id_]() mutable {
        if (engine) {
          engine->DispatchPointerDataPacket(std::move(packet), flow_id);
        }
      }));
}

}  // namespace flutter
