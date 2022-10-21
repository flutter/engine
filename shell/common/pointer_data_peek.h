// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef POINTER_DATA_PEEK_H_
#define POINTER_DATA_PEEK_H_

#include "flutter/runtime/runtime_controller.h"
#include "flutter/shell/common/animator.h"

namespace flutter {

// TODO should give enable/disable flags as well
class PointerDataPeek {
 public:
  PointerDataPeek() {}

  int64_t AddPending(const PointerDataPacket& packet);
  void RemovePending(int64_t id);
  std::unique_ptr<PointerDataPacket> Peek();

 private:
  std::mutex mutex_;
  int next_id_{0};
  std::map<int64_t, PointerDataPacket> pending_packets_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(PointerDataPeek);
};

}  // namespace flutter

#endif  // POINTER_DATA_PEEK_H_
