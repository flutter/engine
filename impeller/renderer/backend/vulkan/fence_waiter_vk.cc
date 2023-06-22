// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/fence_waiter_vk.h"

#include <chrono>

#include "flutter/fml/thread.h"
#include "flutter/fml/trace_event.h"

namespace impeller {

FenceWaiterVK::FenceWaiterVK(std::weak_ptr<DeviceHolder> device_holder)
    : device_holder_(std::move(device_holder)) {
  waiter_thread_ = std::make_unique<std::thread>([&]() { Main(); });
  is_valid_ = true;
}

FenceWaiterVK::~FenceWaiterVK() {
  Terminate();
  if (waiter_thread_) {
    waiter_thread_->join();
  }
}

bool FenceWaiterVK::IsValid() const {
  return is_valid_;
}

bool FenceWaiterVK::AddFence(vk::UniqueFence fence,
                             const fml::closure& callback) {
  TRACE_EVENT0("flutter", "FenceWaiterVK::AddFence");
  if (!IsValid() || !fence || !callback) {
    return false;
  }
  {
    std::scoped_lock lock(wait_set_mutex_);
    wait_set_.push_back(std::move(fence));
    wait_set_callbacks_.push_back(callback);
  }
  wait_set_cv_.notify_one();
  return true;
}

void FenceWaiterVK::Main() {
  fml::Thread::SetCurrentThreadName(
      fml::Thread::ThreadConfig{"io.flutter.impeller.fence_waiter"});

  using namespace std::literals::chrono_literals;

  while (true) {
    std::unique_lock lock(wait_set_mutex_);

    wait_set_cv_.wait(lock, [&]() { return !wait_set_.empty() || terminate_; });

    auto device_holder = device_holder_.lock();
    if (!device_holder || terminate_) {
      break;
    }

    std::vector<vk::UniqueFence> temp_wait_set = std::move(wait_set_);
    std::vector<fml::closure> callbacks = std::move(wait_set_callbacks_);
    lock.unlock();

    std::vector<vk::Fence> fences(temp_wait_set.size());
    for (auto i = 0u; i < temp_wait_set.size(); i++) {
      fences[i] = temp_wait_set[i].get();
    }

    if (fences.empty()) {
      continue;
    }

    auto result = device_holder->GetDevice().waitForFences(
        fences.size(),                           // fences count
        fences.data(),                           // fences
        true,                                    // wait for all
        std::chrono::nanoseconds{100ms}.count()  // timeout (ns)
    );
    if (!(result == vk::Result::eSuccess || result == vk::Result::eTimeout)) {
      break;
    }
    for (auto cb : callbacks) {
      cb();
    }
  }
}

void FenceWaiterVK::Terminate() {
  {
    std::scoped_lock lock(wait_set_mutex_);
    terminate_ = true;
  }
  wait_set_cv_.notify_one();
}

}  // namespace impeller
