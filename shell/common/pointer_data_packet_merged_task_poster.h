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
};

}  // namespace flutter

#endif  // POINTER_DATA_PACKET_MERGED_TASK_POSTER_H_
