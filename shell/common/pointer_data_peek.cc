// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pointer_data_peek.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/common/pointer_data_dispatcher.h"

namespace flutter {

int64_t PointerDataPeek::AddPending(const PointerDataPacket& packet) {
  std::scoped_lock state_lock(mutex_);

  int64_t chosen_id = next_id_++;
  pending_packets_.emplace(
      std::piecewise_construct, std::forward_as_tuple(chosen_id),
      std::forward_as_tuple(packet.data().data(), packet.data().size()));
  return chosen_id;
}

void PointerDataPeek::RemovePending(int64_t id) {
  std::scoped_lock state_lock(mutex_);

  pending_packets_.erase(id);
}

std::unique_ptr<PointerDataPacket> PointerDataPeek::Peek() {
  // TODO should not lock for this long
  std::scoped_lock state_lock(mutex_);

  // TODO quite expensive copy/resize, only for prototype, should optimize later
  std::vector<uint8_t> total_buffer;
  for (auto const& [_, packet] : pending_packets_) {
    const std::vector<uint8_t>& packet_buffer = packet.data();
    total_buffer.insert(total_buffer.end(), packet_buffer.begin(),
                        packet_buffer.end());
  }

  // clear
  pending_packets_.clear();

  return std::make_unique<PointerDataPacket>(total_buffer.data(),
                                             total_buffer.size());
}

}  // namespace flutter
