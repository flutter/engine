// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_
#define POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_

#include "engine.h"

namespace flutter {

class PointerDataPacketMergedTaskPoster {
 public:
  void Dispatch(std::unique_ptr<PointerDataPacket> packet,
                uint64_t flow_id,
                const TaskRunners& task_runners,
                const fml::WeakPtr<Engine>& weak_engine);
};

}  // namespace flutter

#endif  // POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_
