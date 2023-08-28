// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_pool_vk.h"

#include <map>
#include <unordered_map>
#include <vector>

#include "flutter/fml/thread_local.h"
#include "fml/trace_event.h"
#include "impeller/base/thread.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"

namespace impeller {

using CommandPoolMap = std::map<uint64_t, std::shared_ptr<CommandPoolVK>>;
FML_THREAD_LOCAL fml::ThreadLocalUniquePtr<CommandPoolMap> tls_command_pool;

static Mutex g_recycled_pools_mutex;
static std::vector<std::shared_ptr<CommandPoolVK>> g_recycled_pools
    IPLR_GUARDED_BY(g_recycled_pools_mutex);

static Mutex g_all_pools_mutex;
static std::unordered_map<const ContextVK*,
                          std::vector<std::weak_ptr<CommandPoolVK>>>
    g_all_pools IPLR_GUARDED_BY(g_all_pools_mutex);

void CommandPoolVK::NextFrame() {
  if (tls_command_pool.get() == nullptr) {
    return;
  }
  CommandPoolMap& pool_map = *tls_command_pool.get();
  Lock lock(g_recycled_pools_mutex);
  for (auto [key, pool] : pool_map) {
    pool->ConditionalReset();
    g_recycled_pools.push_back(std::move(pool));
  }
  pool_map.clear();
}

std::shared_ptr<CommandPoolVK> CommandPoolVK::GetThreadLocal(
    const ContextVK* context) {
  if (!context) {
    return nullptr;
  }
  if (tls_command_pool.get() == nullptr) {
    tls_command_pool.reset(new CommandPoolMap());
  }
  CommandPoolMap& pool_map = *tls_command_pool.get();
  auto found = pool_map.find(context->GetHash());
  if (found != pool_map.end() && found->second->IsValid()) {
    return found->second;
  }
  std::shared_ptr<CommandPoolVK> pool;
  bool recycled_pool = false;
  {
    Lock lock(g_recycled_pools_mutex);
    int32_t index = -1;
    auto count = 0u;
    for (auto maybe_pool : g_recycled_pools) {
      if (maybe_pool->allocated_count_ == 0u) {
        pool = std::move(maybe_pool);
        index = count;
        recycled_pool = true;
      }
      count++;
    }
    if (index != -1) {
      g_recycled_pools.erase(g_recycled_pools.begin() + index);
    }
  }
  if (!pool) {
    pool = std::shared_ptr<CommandPoolVK>(
        new CommandPoolVK(context, context->GetConcurrentWorkerTaskRunner()));
    if (!pool->IsValid()) {
      return nullptr;
    }
  }
  pool_map[context->GetHash()] = pool;
  if (!recycled_pool) {
    {
      Lock pool_lock(g_all_pools_mutex);
      g_all_pools[context].push_back(pool);
    }
  }
  return pool;
}

void CommandPoolVK::ConditionalReset() {
  ready_for_reset_ = true;
  if (allocated_count_ == buffers_to_collect_.size()) {
    Reset();
  }
}

void CommandPoolVK::ClearAllPools(const ContextVK* context) {
  if (tls_command_pool.get()) {
    tls_command_pool.get()->erase(context->GetHash());
  }
  Lock pool_lock(g_all_pools_mutex);
  if (auto found = g_all_pools.find(context); found != g_all_pools.end()) {
    for (auto& weak_pool : found->second) {
      auto pool = weak_pool.lock();
      if (!pool) {
        // The pool has already died because the thread died.
        continue;
      }
      // The pool is reset but its reference in the TLS map remains till the
      // thread dies.
      pool->Reset();
    }
    g_all_pools.erase(found);
  }
}

CommandPoolVK::CommandPoolVK(const ContextVK* context,
                             std::shared_ptr<fml::ConcurrentTaskRunner> loop)
    : loop_(loop) {
  vk::CommandPoolCreateInfo pool_info;

  pool_info.queueFamilyIndex = context->GetGraphicsQueue()->GetIndex().family;
  pool_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
  auto pool = context->GetDevice().createCommandPoolUnique(pool_info);
  if (pool.result != vk::Result::eSuccess) {
    return;
  }

  device_holder_ = context->GetDeviceHolder();
  graphics_pool_ = std::move(pool.value);
  is_valid_ = true;
}

CommandPoolVK::~CommandPoolVK() = default;

bool CommandPoolVK::IsValid() const {
  return is_valid_;
}

void CommandPoolVK::Reset() {
  // Reset Pool.
  loop_->PostTask([&] {
    TRACE_EVENT0("Flutter", "CommandPoolVK::Reset");
    std::shared_ptr<const DeviceHolder> strong_device = device_holder_.lock();
    if (!strong_device) {
      return;
    }
    strong_device->GetDevice().resetCommandPool(graphics_pool_.get());
    allocated_count_ = 0u;
    buffers_to_collect_.clear();
    ready_for_reset_ = false;
  });
}

vk::CommandPool CommandPoolVK::GetGraphicsCommandPool() const {
  return graphics_pool_.get();
}

vk::UniqueCommandBuffer CommandPoolVK::CreateGraphicsCommandBuffer() {
  TRACE_EVENT0("Flutter", "CommandPoolVK::CreateGraphicsCommandBuffer");
  std::shared_ptr<const DeviceHolder> strong_device = device_holder_.lock();
  if (!strong_device) {
    return {};
  }

  vk::CommandBufferAllocateInfo alloc_info;
  alloc_info.commandPool = graphics_pool_.get();
  alloc_info.commandBufferCount = 1u;
  alloc_info.level = vk::CommandBufferLevel::ePrimary;
  auto [result, buffers] =
      strong_device->GetDevice().allocateCommandBuffersUnique(alloc_info);
  if (result != vk::Result::eSuccess) {
    return {};
  }
  allocated_count_++;
  return std::move(buffers[0]);
}

void CommandPoolVK::CollectGraphicsCommandBuffer(
    vk::UniqueCommandBuffer buffer) {
  if (!graphics_pool_) {
    // If the command pool has already been destroyed, then its command buffers
    // have been freed and are now invalid.
    buffer.release();
  }
  buffers_to_collect_.emplace_back(std::move(buffer));
  if (ready_for_reset_ && allocated_count_ == buffers_to_collect_.size()) {
    Reset();
  }
}

}  // namespace impeller
