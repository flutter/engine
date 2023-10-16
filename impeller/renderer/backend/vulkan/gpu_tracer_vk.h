// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/device_holder.h"
#include "vulkan/vulkan_handles.hpp"

namespace impeller {

/// @brief A class that uses timestamp queries to record the approximate GPU
/// execution time.
class GPUTracerVK {
 public:
  explicit GPUTracerVK(const std::shared_ptr<DeviceHolder>& device_holder);

  ~GPUTracerVK() = default;

  /// @brief Record a timestamp query into the provided cmd buffer to record
  ///        start time.
  void RecordCmdBufferStart(const vk::CommandBuffer& buffer);

  /// @brief Record a timestamp query into the provided cmd buffer to record end
  ///        time.
  ///
  ///        Returns the index that should be passed to [OnFenceComplete].
  size_t RecordCmdBufferEnd(const vk::CommandBuffer& buffer);

  /// @brief Signal that the cmd buffer is completed.
  void OnFenceComplete(size_t frame_index);

  /// @brief Signal the start of a frame workload.
  ///
  ///        Any cmd buffers that are created after this call and before
  ///        [MarkFrameEnd] will be attributed to the current frame.
  void MarkFrameStart();

  /// @brief Signal the end of a frame workload.
  void MarkFrameEnd();

 private:
  const std::shared_ptr<DeviceHolder> device_holder_;

  struct GPUTraceState {
    size_t current_index = 0;
    size_t pending_buffers = 0;
    vk::UniqueQueryPool query_pool;
  };

  mutable Mutex trace_state_mutex_;
  GPUTraceState trace_states_[16] IPLR_GUARDED_BY(trace_state_mutex_);
  size_t current_state_ IPLR_GUARDED_BY(trace_state_mutex_) = 0u;

  // The number of nanoseconds for each timestamp unit.
  float timestamp_period_ = 1;
  bool in_frame_ = false;
  bool valid_ = false;
};

}  // namespace impeller
