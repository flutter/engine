// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_MERGED_DISPATCHER_H_
#define FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_MERGED_DISPATCHER_H_

#include <cstring>
#include <vector>

namespace flutter {

class PointerDataPacketMergedDispatcher {
  void Dispatch(std::unique_ptr<PointerDataPacket> packet,
                uint64_t flow_id,
                const TaskRunners& task_runners,
                const fml::WeakPtr<Engine>& weak_engine);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_MERGED_DISPATCHER_H_
