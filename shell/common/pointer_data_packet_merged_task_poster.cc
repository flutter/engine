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
  int previous_pending_packet_count = pending_packet_count_.fetch_add(1);

  if (previous_pending_packet_count > 0) {
    // already have pending PostTask, so put data to sidecar_packets_
    std::scoped_lock lock(mutex_);
    sidecar_packets_.push_back(std::move(packet));
  } else {
    task_runner->PostTask(fml::MakeCopyable([main_packet = std::move(packet),
                                             callback, this]() mutable {
      // TODO reason about mutex and atomic variable ordering
      int previous_pending_packet_count = pending_packet_count_.exchange(0);

      if (previous_pending_packet_count <= 1) {
        // the shortcut - only have the main packet, no sidecar packets
        callback(std::move(main_packet));
      } else {
        std::vector<std::unique_ptr<PointerDataPacket>> moved_sidecar_packets;
        {
          std::scoped_lock lock(mutex_);
          moved_sidecar_packets.swap(sidecar_packets_);
        }

        std::vector<uint8_t> gathered_buffer;
        gathered_buffer.insert(gathered_buffer.end(),
                               main_packet->data().begin(),
                               main_packet->data().end());
        for (const std::unique_ptr<PointerDataPacket>& packet :
             moved_sidecar_packets) {
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
