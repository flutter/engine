// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <set>
#include <thread>

#include "flutter/fml/macros.h"
#include "fml/concurrent_message_loop.h"
#include "impeller/base/thread.h"
#include "impeller/renderer/backend/vulkan/device_holder.h"
#include "impeller/renderer/backend/vulkan/shared_object_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller {

class ContextVK;

class CommandPoolVK : public std::enable_shared_from_this<CommandPoolVK> {
 public:
  static std::shared_ptr<CommandPoolVK> GetThreadLocal(
      const ContextVK* context);

  static void ClearAllPools(const ContextVK* context);

  static void NextFrame();

  ~CommandPoolVK();

  bool IsValid() const;

  void Reset();

  vk::CommandPool GetGraphicsCommandPool() const;

  vk::UniqueCommandBuffer CreateGraphicsCommandBuffer();

  void CollectGraphicsCommandBuffer(vk::UniqueCommandBuffer buffer);

  void ConditionalReset();

 private:
  std::weak_ptr<const DeviceHolder> device_holder_;
  vk::UniqueCommandPool graphics_pool_;
  std::vector<vk::UniqueCommandBuffer> buffers_to_collect_;
  std::atomic<size_t> allocated_count_ = 0u;
  std::shared_ptr<fml::ConcurrentTaskRunner> loop_;
  bool ready_for_reset_ = false;
  bool is_valid_ = false;

  explicit CommandPoolVK(const ContextVK* context,
                         std::shared_ptr<fml::ConcurrentTaskRunner> loop);

  FML_DISALLOW_COPY_AND_ASSIGN(CommandPoolVK);
};

}  // namespace impeller
