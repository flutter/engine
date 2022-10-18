// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pointer_data_packet_merged_task_poster.h"
#include "flutter/fml/make_copyable.h"

namespace flutter {

void PointerDataPacketMergedTaskPoster::Dispatch(
    std::unique_ptr<PointerDataPacket> packet,
    const fml::RefPtr<fml::TaskRunner>& task_runner,
    const PointerDataPacketMergedTaskPoster::Callback& callback) {
  bool previous_pending_post_task;
  {
    std::scoped_lock lock(mutex_);

    previous_pending_post_task = pending_post_task_;
    pending_post_task_ = true;

    if (previous_pending_post_task) {
      sidecar_packets_.push_back(std::move(packet));
    }
  }

  if (!previous_pending_post_task) {
    task_runner->PostTask(fml::MakeCopyable([main_packet = std::move(packet),
                                             callback, this]() mutable {
      std::optional<std::vector<std::unique_ptr<PointerDataPacket>>>
          moved_sidecar_packets;
      {
        std::scoped_lock lock(mutex_);
        pending_post_task_ = false;
        if (!sidecar_packets_.empty()) {
          moved_sidecar_packets =
              std::vector<std::unique_ptr<PointerDataPacket>>{};
          moved_sidecar_packets->swap(sidecar_packets_);
        }
      }

      if (!moved_sidecar_packets.has_value()) {
        // the shortcut
        callback(std::move(main_packet));
      } else {
        std::vector<uint8_t> gathered_buffer;
        gathered_buffer.insert(gathered_buffer.end(),
                               main_packet->data().begin(),
                               main_packet->data().end());
        for (const std::unique_ptr<PointerDataPacket>& packet :
             moved_sidecar_packets.value()) {
          gathered_buffer.insert(gathered_buffer.end(), packet->data().begin(),
                                 packet->data().end());
        }

        std::unique_ptr<PointerDataPacket> gathered_packet =
            std::make_unique<PointerDataPacket>(gathered_buffer.data(),
                                                gathered_buffer.size());

        callback(std::move(gathered_packet));
      }
    }));
  }
}

}  // namespace flutter
