// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_pool_recycler_vk.h"
#include "fml/closure.h"
#include "impeller/renderer/backend/vulkan/command_pool_vk.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"

namespace impeller {

// For each thread, we keep a map of context hashes to command pools.
thread_local std::map<uint64_t, std::shared_ptr<CommandPoolVK>> command_pools;

std::shared_ptr<CommandPoolVK> CommandPoolRecyclerVK::Get() {
  auto const hash = context_->GetHash();
  auto const found = command_pools.find(hash);
  if (found != command_pools.end()) {
    return found->second;
  }
  auto const pool = CreateOrReusePool();
  command_pools[hash] = pool;
  return pool;
}

void CommandPoolRecyclerVK::Recycle() {
  // Check if we have a pool for this thread.
  auto const hash = context_->GetHash();
  auto const found = command_pools.find(hash);
  if (found == command_pools.end()) {
    return;
  }

  // Recycle the pool in a background thread.
  auto const pool = found->second;

  // A callback that will be run in a background thread to reset the pool.
  fml::ScopedCleanupClosure reset([this, &pool]() {
    pool->Reset();
    recycled_pools_.push_back(pool);
  });

  // This will immediately fall out of scope, and moved to a background thread
  // where it will be run.
  UniqueResourceVKT<fml::ScopedCleanupClosure>(
      /*resource_manager=*/context_->GetResourceManager(),
      /*resource=*/std::move(reset));
}

std::shared_ptr<CommandPoolVK> CommandPoolRecyclerVK::CreateOrReusePool() {
  // If there is a recycled pool available, use it.
  if (!recycled_pools_.empty()) {
    auto const pool = recycled_pools_.back();
    recycled_pools_.pop_back();
    return pool;
  }

  // Otherwise, create a new pool.
  return std::make_shared<CommandPoolVK>(context_);
}

}  // namespace impeller
