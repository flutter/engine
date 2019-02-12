// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_vm_lifecycle.h"

#include <mutex>

// The use of std::lock causes a false warning because that method has not been
// decorated with thread safety annotations.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wthread-safety-analysis"

namespace blink {

static std::mutex gLifecycleMutex;
// TODO: This needs to be a shared exclusive lock.
static std::recursive_mutex gAccessMutex;
std::weak_ptr<DartVM> gVM;
// TODO: Move the VM launch count global here.

DartVMLifecycleReference::DartVMLifecycleReference(std::shared_ptr<DartVM> vm)
    : vm_(vm) {}

DartVMLifecycleReference::DartVMLifecycleReference(
    DartVMLifecycleReference&& other) {
  other.abandoned_ = true;
  vm_ = std::move(other.vm_);
}

DartVMLifecycleReference::~DartVMLifecycleReference() {
  if (abandoned_) {
    return;
  }
  std::lock(gLifecycleMutex, gAccessMutex);
  std::lock_guard<std::mutex> lifecycle_lock(gLifecycleMutex, std::adopt_lock);
  std::lock_guard<std::recursive_mutex> access_lock(gAccessMutex,
                                                    std::adopt_lock);
  vm_.reset();
}

DartVMLifecycleReference DartVMLifecycleReference::Create(
    Settings settings,
    fml::RefPtr<DartSnapshot> vm_snapshot,
    fml::RefPtr<DartSnapshot> isolate_snapshot,
    fml::RefPtr<DartSnapshot> shared_snapshot) {
  std::lock(gLifecycleMutex, gAccessMutex);

  std::lock_guard<std::mutex> lifecycle_lock(gLifecycleMutex, std::adopt_lock);
  std::lock_guard<std::recursive_mutex> access_lock(gAccessMutex,
                                                    std::adopt_lock);

  // If there is already a running VM in the process. Grab a strong reference to
  // it.
  if (auto vm = gVM.lock()) {
    FML_LOG(INFO) << "Attempted to create a VM in a process where one was "
                     "already running. Ignoring arguments for current VM "
                     "create call and reusing the old VM.";
    // There was already a running VM in the process,
    return DartVMLifecycleReference{std::move(vm)};
  }

  // If there is no VM in the process. Initialize one, hold the weak reference
  // and pass a strong reference to the caller.
  auto vm =
      DartVM::Create(std::move(settings), std::move(vm_snapshot),
                     std::move(isolate_snapshot), std::move(shared_snapshot));
  gVM = vm;
  return DartVMLifecycleReference{std::move(vm)};
}

DartVMAccessReference::DartVMAccessReference(std::shared_ptr<DartVM> vm)
    : vm_(std::move(vm)) {}

DartVMAccessReference::DartVMAccessReference(DartVMAccessReference&& other) {
  other.abandoned_ = true;
  vm_ = std::move(other.vm_);
}

DartVMAccessReference::~DartVMAccessReference() {
  if (abandoned_) {
    return;
  }
  gAccessMutex.unlock();  // Locked in the |DartVMAccessReference::Create|.
}

DartVMAccessReference DartVMAccessReference::Create() {
  gAccessMutex.lock();  // Unlocked in the |~DartVMAccessReference|.
  return DartVMAccessReference{gVM.lock()};
}

}  // namespace blink
