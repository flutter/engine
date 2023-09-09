// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_pool_recycler_vk.h"
#include <memory>
#include <optional>
#include "fml/trace_event.h"

namespace impeller {

CommandPoolResourceVK::~CommandPoolResourceVK() {
  auto recycler = recycler_.lock();
  if (!recycler) {
    return;
  }
  recycler->Reclaim(std::move(pool_));
}

// Associates a resource with a thread and context.
thread_local std::unordered_map<uint64_t,
                                std::shared_ptr<CommandPoolResourceVK>>
    resources_;

// TODO(matanlurey): Return a status_or<> instead of nullptr when we have one.
std::shared_ptr<CommandPoolResourceVK> CommandPoolRecyclerVK::Get() {
  auto const strong_context = context_.lock();
  if (!strong_context) {
    return nullptr;
  }

  // If there is a resource in used for this thread and context, return it.
  auto const hash = strong_context->GetHash();
  auto const it = resources_.find(hash);
  if (it != resources_.end()) {
    return it->second;
  }

  // Otherwise, create a new resource and return it.
  auto const pool = Create();
  if (!pool) {
    return nullptr;
  }
  auto const result = std::make_shared<CommandPoolResourceVK>(std::move(*pool),
                                                              weak_from_this());
  resources_[hash] = result;
  return result;
}

// TODO(matanlurey): Return a status_or<> instead of nullopt when we have one.
std::optional<vk::UniqueCommandPool> CommandPoolRecyclerVK::Create() {
  // If we can reuse a command pool, do so.
  if (auto pool = Reuse()) {
    return pool;
  }

  // Otherwise, create a new one.
  return std::nullopt;
}

std::optional<vk::UniqueCommandPool> CommandPoolRecyclerVK::Reuse() {
  // If there are no recycled pools, return nullopt.
  Lock _(recycled_mutex_);
  if (recycled_.empty()) {
    return std::nullopt;
  }

  // Otherwise, remove and return a recycled pool.
  auto pool = std::move(recycled_.back());
  recycled_.pop_back();
  return std::move(pool);
}

void CommandPoolRecyclerVK::Reclaim(vk::UniqueCommandPool&& pool) {
  TRACE_EVENT0("flutter", "ReclaimCommandPool");
  // FIXME: Assert that this is called on a background thread.

  // Reset the pool on a background thread.
  auto strong_context = context_.lock();
  if (!strong_context) {
    return;
  }
  auto device = strong_context->GetDevice();
  device.resetCommandPool(pool.get());

  // Move the pool to the recycled list.
  Lock _(recycled_mutex_);
  recycled_.push_back(std::move(pool));
}

void CommandPoolRecyclerVK::Recycle() {
  resources_.clear();
}

}  // namespace impeller
