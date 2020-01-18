// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/pointer_data_dispatcher.h"

namespace flutter {

PointerDataDispatcher::~PointerDataDispatcher() = default;
DefaultPointerDataDispatcher::~DefaultPointerDataDispatcher() = default;

SmoothPointerDataDispatcher::SmoothPointerDataDispatcher(Delegate& delegate)
    : DefaultPointerDataDispatcher(delegate), weak_factory_(this) {}
SmoothPointerDataDispatcher::~SmoothPointerDataDispatcher() = default;

void DefaultPointerDataDispatcher::DispatchPacket(
    std::unique_ptr<PointerDataPacket> packet,
    uint64_t trace_flow_id) {
  delegate_.DoDispatchPacket(std::move(packet), trace_flow_id);
}

void SmoothPointerDataDispatcher::DispatchPacket(
    std::unique_ptr<PointerDataPacket> packet,
    uint64_t trace_flow_id) {
  if (is_pointer_data_in_progress_) {
    if (pending_packet_ != nullptr) {
      DispatchPendingPacket();
    }
    pending_packet_ = std::move(packet);
    pending_trace_flow_id_ = trace_flow_id;
  } else {
    FML_DCHECK(pending_packet_ == nullptr);
    DefaultPointerDataDispatcher::DispatchPacket(std::move(packet),
                                                 trace_flow_id);
  }
  is_pointer_data_in_progress_ = true;
  ScheduleSecondaryVsyncCallback();
}

void SmoothPointerDataDispatcher::ScheduleSecondaryVsyncCallback() {
  delegate_.ScheduleSecondaryVsyncCallback(
      [dispatcher = weak_factory_.GetWeakPtr()](fml::TimePoint,
                                                fml::TimePoint) {
        if (dispatcher && dispatcher->is_pointer_data_in_progress_) {
          if (dispatcher->pending_packet_ != nullptr) {
            dispatcher->DispatchPendingPacket();
          } else {
            dispatcher->is_pointer_data_in_progress_ = false;
          }
        }
      });
}

void SmoothPointerDataDispatcher::DispatchPendingPacket() {
  FML_DCHECK(pending_packet_ != nullptr);
  FML_DCHECK(is_pointer_data_in_progress_);
  DefaultPointerDataDispatcher::DispatchPacket(std::move(pending_packet_),
                                               pending_trace_flow_id_);
  pending_packet_ = nullptr;
  pending_trace_flow_id_ = -1;
  ScheduleSecondaryVsyncCallback();
}

ResamplingPointerDataDispatcher::ResamplingPointerDataDispatcher(
    Delegate& delegate,
    int64_t sampling_offset_us)
    : DefaultPointerDataDispatcher(delegate),
      sampling_offset_us_(sampling_offset_us),
      weak_factory_(this) {}
ResamplingPointerDataDispatcher::~ResamplingPointerDataDispatcher() = default;

void ResamplingPointerDataDispatcher::DispatchPacket(
    std::unique_ptr<PointerDataPacket> packet,
    uint64_t trace_flow_id) {
  TRACE_EVENT0("flutter", "ResamplingPointerDataDispatcher::DispatchPacket");
  size_t pointer_data_count = packet->data().size() / sizeof(PointerData);
  for (size_t i = 0; i < pointer_data_count; i++) {
    PointerEvent event;
    event.trace_flow_id = trace_flow_id;
    packet->GetPointerData(i, &event.data);
    auto trace_arg = std::to_string(event.data.time_stamp);
    TRACE_EVENT1("flutter", "PointerEvent", "time_stamp", trace_arg.c_str());
    pending_events_.push_back(event);
  }
  TRACE_FLOW_STEP("flutter", "PointerEvent", trace_flow_id);
  DispatchPendingPackets();
}

void ResamplingPointerDataDispatcher::ScheduleSecondaryVsyncCallback() {
  if (secondary_vsync_callback_pending_) {
    return;
  }
  secondary_vsync_callback_pending_ = true;
  delegate_.ScheduleSecondaryVsyncCallback(
      [dispatcher = weak_factory_.GetWeakPtr()](
          fml::TimePoint frame_start_time, fml::TimePoint frame_target_time) {
        if (dispatcher) {
          auto trace_arg =
              std::to_string(frame_start_time.ToEpochDelta().ToMicroseconds());
          TRACE_EVENT1("flutter",
                       "ResamplingPointerDataDispatcher::VsyncCallback",
                       "frame_start_time_us", trace_arg.c_str());
          dispatcher->sample_time_us_ =
              frame_start_time.ToEpochDelta().ToMicroseconds() +
              dispatcher->sampling_offset_us_;
          dispatcher->secondary_vsync_callback_pending_ = false;
          dispatcher->DispatchPendingPackets();
        }
      });
}

void ResamplingPointerDataDispatcher::DispatchPendingPackets() {
  TRACE_EVENT0("flutter",
               "ResamplingPointerDataDispatcher::DispatchPendingPackets");
  ConsumePendingEvents();
  UpdateDownPointers();
  DispatchMoveChanges();
  // Schedule vsync callback if down pointers exists or events are
  // still pending.
  if (!pending_events_.empty() || !down_pointers_.empty()) {
    ScheduleSecondaryVsyncCallback();
  }
}

void ResamplingPointerDataDispatcher::ConsumePendingEvents() {
  while (!pending_events_.empty()) {
    auto& event = pending_events_.front();

    // Stop consuming events if more recent than current sample time.
    if (event.data.time_stamp > sample_time_us_) {
      return;
    }

    int device = event.data.device;
    switch (event.data.change) {
      case flutter::PointerData::Change::kDown: {
        DownPointer pointer;
        pointer.last = event;
        pointer.next = event;
        down_pointers_[device] = pointer;
      } break;
      case flutter::PointerData::Change::kMove:
        down_pointers_[device].next = event;
        break;
      case flutter::PointerData::Change::kCancel:
      case flutter::PointerData::Change::kUp:
        down_pointers_.erase(device);
        break;
      case flutter::PointerData::Change::kAdd:
      case flutter::PointerData::Change::kRemove:
      case flutter::PointerData::Change::kHover:
        break;
    }

    // Dispatch non-move changes without re-sampling.
    if (event.data.change != flutter::PointerData::Change::kMove) {
      TRACE_FLOW_STEP("flutter", "PointerEvent", event.trace_flow_id);
      auto packet = std::make_unique<flutter::PointerDataPacket>(1);
      packet->SetPointerData(0, event.data);
      DefaultPointerDataDispatcher::DispatchPacket(
          pointer_data_packet_converter_.Convert(std::move(packet)),
          event.trace_flow_id);
    }

    pending_events_.pop_front();
  }
}

void ResamplingPointerDataDispatcher::UpdateDownPointers() {
  // Update |down_pointers_| by examining pending changes.
  for (auto& event : pending_events_) {
    auto it = down_pointers_.find(event.data.device);
    if (it != down_pointers_.end()) {
      switch (event.data.change) {
        case flutter::PointerData::Change::kDown:
        case flutter::PointerData::Change::kMove:
        case flutter::PointerData::Change::kCancel:
        case flutter::PointerData::Change::kUp:
          // Update next event if not already passed sample time.
          if (it->second.next.data.time_stamp < sample_time_us_) {
            it->second.next = event;
          }
          break;
        case flutter::PointerData::Change::kAdd:
        case flutter::PointerData::Change::kRemove:
        case flutter::PointerData::Change::kHover:
          break;
      }
    }
  }
}

void ResamplingPointerDataDispatcher::DispatchMoveChanges() {
  // Dispatch move changes for |down_pointers_|.
  for (auto& it : down_pointers_) {
    DownPointer& p = it.second;
    PointerData data = p.next.data;
    // Re-sample if next time stamp is past sample time.
    if (p.next.data.time_stamp > sample_time_us_ &&
        p.next.data.time_stamp > p.last.data.time_stamp) {
      double interval =
          static_cast<double>(p.next.data.time_stamp - p.last.data.time_stamp);
      double scalar = (sample_time_us_ - p.last.data.time_stamp) / interval;
      data.physical_x =
          p.last.data.physical_x +
          (p.next.data.physical_x - p.last.data.physical_x) * scalar;
      data.physical_y =
          p.last.data.physical_y +
          (p.next.data.physical_y - p.last.data.physical_y) * scalar;
      data.time_stamp = sample_time_us_;
      auto trace_arg1 = std::to_string(p.last.data.time_stamp);
      auto trace_arg2 = std::to_string(p.next.data.time_stamp);
      TRACE_EVENT2("flutter", "ResampledPointerEvent", "last_time_stamp_us",
                   trace_arg1.c_str(), "next_time_stamp_us",
                   trace_arg2.c_str());
    }

    // Dispatch move change if time stamp is greater than last event.
    if (data.time_stamp > p.last.data.time_stamp) {
      data.change = flutter::PointerData::Change::kMove;
      TRACE_FLOW_STEP("flutter", "PointerEvent", p.next.trace_flow_id);
      auto packet = std::make_unique<flutter::PointerDataPacket>(1);
      packet->SetPointerData(0, data);
      DefaultPointerDataDispatcher::DispatchPacket(
          pointer_data_packet_converter_.Convert(std::move(packet)),
          p.next.trace_flow_id);
      // Update last event.
      p.last.data = data;
    }
  }
}

}  // namespace flutter
