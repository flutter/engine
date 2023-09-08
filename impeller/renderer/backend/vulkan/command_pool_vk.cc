// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_pool_vk.h"

#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

#include "flutter/fml/thread_local.h"
#include "impeller/base/thread.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"

namespace impeller {

using CommandPoolMap = std::map<uint64_t, std::shared_ptr<CommandPoolVK>>;
// NOTE: How TF do we reset multiple threads?
FML_THREAD_LOCAL fml::ThreadLocalUniquePtr<CommandPoolMap> tls_command_pool;

// static Mutex g_all_pools_mutex;
// static std::unordered_map<const ContextVK*,
//                           std::vector<std::weak_ptr<CommandPoolVK>>>
//     g_all_pools IPLR_GUARDED_BY(g_all_pools_mutex);

void CommandPoolVK::ReleaseAllPools(const ContextVK* context) {
  if (tls_command_pool.get() == nullptr) {
    return;
  }
  CommandPoolMap& pool_map = *tls_command_pool.get();
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
  auto pool = CommandPoolVK::FromContext(context);
  if (!pool || !pool->IsValid()) {
    return nullptr;
  }
  pool_map[context->GetHash()] = pool;
  // {
  //   Lock pool_lock(g_all_pools_mutex);
  //   g_all_pools[context].push_back(pool);
  // }
  return pool;
}

void CommandPoolVK::ClearAllPools(const ContextVK* context) {
  // if (tls_command_pool.get()) {
  //   tls_command_pool.get()->erase(context->GetHash());
  // }
  // Lock pool_lock(g_all_pools_mutex);
  // if (auto found = g_all_pools.find(context); found != g_all_pools.end()) {
  //   for (auto& weak_pool : found->second) {
  //     auto pool = weak_pool.lock();
  //     if (!pool) {
  //       // The pool has already died because the thread died.
  //       continue;
  //     }
  //     // The pool is reset but its reference in the TLS map remains till the
  //     // thread dies.
  //     pool->Reset();
  //   }
  //   g_all_pools.erase(found);
  // }
}

std::shared_ptr<CommandPoolVK> CommandPoolVK::FromContext(
    const ContextVK* context) {
  auto pool = context->GetCommandPool();
  if (!pool) {
    return nullptr;
  }

  return std::make_shared<CommandPoolVK>(CommandPoolVK(
      std::this_thread::get_id(), context->weak_from_this(), std::move(*pool)));
}

CommandPoolVK::CommandPoolVK(std::thread::id thread_id,
                             std::weak_ptr<const ContextVK> context,
                             ManagedCommandPoolVK graphics_pool)
    : owner_id_(thread_id),
      context_(std::move(context)),
      graphics_pool_(std::move(graphics_pool)) {
  // FIXME: Not always true.
  is_valid_ = true;
}

struct JustFuckingCompile {
  ManagedCommandPoolVK pool;
  std::vector<vk::UniqueCommandBuffer> buffers;
};

CommandPoolVK::~CommandPoolVK() {
  auto strong_context = context_.lock();
  if (!strong_context) {
    return;
  }
  auto pool = std::move(graphics_pool_);
  UniqueResourceVKT<JustFuckingCompile>(
      strong_context->GetResourceManager(),
      JustFuckingCompile{.pool = std::move(pool),
                         .buffers = std::move(recycled_buffers_)});
}

bool CommandPoolVK::IsValid() const {
  return is_valid_;
}

void CommandPoolVK::Reset() {
  // {
  //   Lock lock(buffers_to_collect_mutex_);
  //   graphics_pool_.reset();

  //   // When the command pool is destroyed, all of its command buffers are
  //   freed.
  //   // Handles allocated from that pool are now invalid and must be
  //   discarded. for (vk::UniqueCommandBuffer& buffer : buffers_to_collect_)
  //   {
  //     buffer.release();
  //   }
  //   buffers_to_collect_.clear();
  // }

  // for (vk::UniqueCommandBuffer& buffer : recycled_buffers_) {
  //   buffer.release();
  // }
  // recycled_buffers_.clear();

  // is_valid_ = false;
}

vk::UniqueCommandBuffer CommandPoolVK::CreateGraphicsCommandBuffer() {
  auto strong_context = context_.lock();
  if (!strong_context) {
    return {};
  }
  FML_DCHECK(std::this_thread::get_id() == owner_id_);
  // {
  //   Lock lock(buffers_to_collect_mutex_);
  //   GarbageCollectBuffersIfAble();
  // }

  // if (!recycled_buffers_.empty()) {
  //   vk::UniqueCommandBuffer result = std::move(recycled_buffers_.back());
  //   recycled_buffers_.pop_back();
  //   return result;
  // }

  vk::CommandBufferAllocateInfo alloc_info;
  alloc_info.commandPool = graphics_pool_.pool_.get();
  alloc_info.commandBufferCount = 1u;
  alloc_info.level = vk::CommandBufferLevel::ePrimary;
  auto [result, buffers] =
      strong_context->GetDevice().allocateCommandBuffersUnique(alloc_info);
  if (result != vk::Result::eSuccess) {
    return {};
  }
  buffer_count_++;
  return std::move(buffers[0]);
}

void CommandPoolVK::CollectGraphicsCommandBuffer(
    vk::UniqueCommandBuffer buffer) {
  buffer_count_--;
  recycled_buffers_.push_back(std::move(buffer));
  // Lock lock(buffers_to_collect_mutex_);
  // if (!graphics_pool_) {
  //   // If the command pool has already been destroyed, then its command
  //   buffers
  //   // have been freed and are now invalid.
  //   buffer.release();
  // }
  // buffers_to_collect_.emplace_back(std::move(buffer));
  // GarbageCollectBuffersIfAble();
}

// void CommandPoolVK::GarbageCollectBuffersIfAble() {
//   // if (std::this_thread::get_id() != owner_id_) {
//   //   return;
//   // }

//   // for (auto& buffer : buffers_to_collect_) {
//   //   buffer->reset();
//   //   recycled_buffers_.emplace_back(std::move(buffer));
//   // }

//   // buffers_to_collect_.clear();
// }

}  // namespace impeller
