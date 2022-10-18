// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_
#define POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_

#include "engine.h"

namespace flutter {

class PointerDataPacketMergedTaskPoster {
 public:
  using Callback = std::function<void(std::unique_ptr<PointerDataPacket>)>;

  void Dispatch(std::unique_ptr<PointerDataPacket> packet,
                const fml::RefPtr<fml::TaskRunner>& task_runner,
                const Callback& callback);

 private:
  std::mutex mutex_;
  std::vector<std::unique_ptr<PointerDataPacket>> sidecar_packets_;
  // 0:  No pending PostTask at all
  // 1:  Has a PostTask, without sidecar_packets_
  // N(>1): Has a PostTask, and have (N-1) sidecar_packets_
  std::atomic<int32_t> pending_packet_count_;
  FML_DISALLOW_COPY_AND_ASSIGN(PointerDataPacketMergedTaskPoster);
};

}  // namespace flutter

#endif  // POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_
