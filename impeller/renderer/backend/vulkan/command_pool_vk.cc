// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_pool_vk.h"
#include <memory>
#include <optional>
#include <utility>
#include "fml/macros.h"
#include "fml/thread_local.h"
#include "fml/trace_event.h"
#include "impeller/renderer/backend/vulkan/resource_manager_vk.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

class BackgroundCommandPoolVK {
 public:
  BackgroundCommandPoolVK(BackgroundCommandPoolVK&&) = default;

  explicit BackgroundCommandPoolVK(
      vk::UniqueCommandPool&& pool,
      std::vector<vk::UniqueCommandBuffer>&& buffers,
      std::weak_ptr<CommandPoolRecyclerVK> recycler)
      : pool_(std::move(pool)),
        buffers_(std::move(buffers)),
        recycler_(std::move(recycler)) {}

  ~BackgroundCommandPoolVK() {
    auto const recycler = recycler_.lock();
    if (!recycler) {
      return;
    }
    recycler->Reclaim(std::move(pool_));
  }

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(BackgroundCommandPoolVK);

  vk::UniqueCommandPool pool_;
  std::vector<vk::UniqueCommandBuffer> buffers_;
  std::weak_ptr<CommandPoolRecyclerVK> recycler_;
};

CommandPoolResourceVK::~CommandPoolResourceVK() {
  auto const context = context_.lock();
  if (!context) {
    return;
  }
  auto const recycler = context->GetCommandPoolRecycler();
  if (!recycler) {
    return;
  }
  UniqueResourceVKT<BackgroundCommandPoolVK> pool(
      context->GetResourceManager(),
      BackgroundCommandPoolVK(std::move(pool_), std::move(collected_buffers_),
                              recycler));
}

// TODO(matanlurey): Return a status_or<> instead of {} when we have one.
vk::UniqueCommandBuffer CommandPoolResourceVK::CreateBuffer() {
  auto const context = context_.lock();
  if (!context) {
    return {};
  }

  auto const device = context->GetDevice();
  vk::CommandBufferAllocateInfo info;
  info.setCommandPool(pool_.get());
  info.setCommandBufferCount(1u);
  info.setLevel(vk::CommandBufferLevel::ePrimary);
  auto [result, buffers] = device.allocateCommandBuffersUnique(info);
  if (result != vk::Result::eSuccess) {
    return {};
  }

  return std::move(buffers[0]);
}

void CommandPoolResourceVK::CollectBuffer(vk::UniqueCommandBuffer&& buffer) {
  collected_buffers_.push_back(std::move(buffer));
}

// Associates a resource with a thread and context.
using CommandPoolMap =
    std::unordered_map<uint64_t, std::shared_ptr<CommandPoolResourceVK>>;
FML_THREAD_LOCAL fml::ThreadLocalUniquePtr<CommandPoolMap> resources_;

// TODO(matanlurey): Return a status_or<> instead of nullptr when we have one.
std::shared_ptr<CommandPoolResourceVK> CommandPoolRecyclerVK::Get() {
  auto const strong_context = context_.lock();
  if (!strong_context) {
    return nullptr;
  }

  // If there is a resource in used for this thread and context, return it.
  auto resources = resources_.get();
  if (!resources) {
    resources = new CommandPoolMap();
    resources_.reset(resources);
  }
  auto map = *resources;
  auto const hash = strong_context->GetHash();
  auto const it = map.find(hash);
  if (it != map.end()) {
    return it->second;
  }

  // Otherwise, create a new resource and return it.
  auto pool = Create();
  if (!pool) {
    return nullptr;
  }

  auto const resource =
      std::make_shared<CommandPoolResourceVK>(std::move(*pool), context_);
  map[hash] = resource;
  return resource;
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
  TRACE_EVENT0("impeller", "ReclaimCommandPool");
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

CommandPoolRecyclerVK::~CommandPoolRecyclerVK() {
  // Ensure all recycled pools are reclaimed before this is destroyed.
  Recycle();
}

void CommandPoolRecyclerVK::Recycle() {
  auto const resources = resources_.get();
  if (!resources) {
    return;
  }
  resources->clear();
}

}  // namespace impeller
