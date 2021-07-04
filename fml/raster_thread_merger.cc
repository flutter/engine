// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/raster_thread_merger.h"

#include "flutter/fml/message_loop_impl.h"

namespace fml {

RasterThreadMerger::RasterThreadMerger(fml::TaskQueueId platform_queue_id,
                                       fml::TaskQueueId gpu_queue_id)
    : platform_queue_id_(platform_queue_id),
      gpu_queue_id_(gpu_queue_id),
      shared_merger_impl_(fml::SharedThreadMergerImpl::GetSharedImpl(platform_queue_id, gpu_queue_id)),
      enabled_(true) {
  FML_LOG(ERROR) << "shared_merger_impl_=" << shared_merger_impl_.get();
}

void RasterThreadMerger::SetMergeUnmergeCallback(const fml::closure& callback) {
  merge_unmerge_callback_ = callback;
}

void RasterThreadMerger::MergeWithLease(size_t lease_term) {
  std::scoped_lock lock(mutex_);
  if (TaskQueuesAreSame()) {
    return;
  }
  if (!IsEnabledUnSafe()) {
    return;
  }
  FML_DCHECK(lease_term > 0) << "lease_term should be positive.";

  if (IsMergedUnSafe()) {
    merged_condition_.notify_one();
    return;
  }

  bool success = shared_merger_impl_->MergeWithLease(lease_term);
  if (success && merge_unmerge_callback_ != nullptr) {
    merge_unmerge_callback_();
  }

  merged_condition_.notify_one();
}

void RasterThreadMerger::UnMergeNowIfLastOne() {
  std::scoped_lock lock(mutex_);

  if (TaskQueuesAreSame()) {
    return;
  }
  if (!IsEnabledUnSafe()) {
    return;
  }
  bool success = shared_merger_impl_->UnMergeNowIfLastOne(this);
  if (success && merge_unmerge_callback_ != nullptr) {
    merge_unmerge_callback_();
  }
}

bool RasterThreadMerger::IsOnPlatformThread() const {
  return MessageLoop::GetCurrentTaskQueueId() == platform_queue_id_;
}

bool RasterThreadMerger::IsOnRasterizingThread() const {
  if (IsMergedUnSafe()) {
    return IsOnPlatformThread();
  } else {
    return !IsOnPlatformThread();
  }
}

void RasterThreadMerger::ExtendLeaseTo(size_t lease_term) {
  FML_DCHECK(lease_term > 0) << "lease_term should be positive.";
  if (TaskQueuesAreSame()) {
    return;
  }
  std::scoped_lock lock(mutex_);
  shared_merger_impl_->ExtendLeaseTo(lease_term);
}

bool RasterThreadMerger::IsMerged() {
  std::scoped_lock lock(mutex_);
  bool is_merged = IsMergedUnSafe();
  return is_merged;
}

void RasterThreadMerger::Enable() {
  std::scoped_lock lock(mutex_);
  enabled_ = true;
}

void RasterThreadMerger::Disable() {
  std::scoped_lock lock(mutex_);
  enabled_ = false;
}

bool RasterThreadMerger::IsEnabled() {
  std::scoped_lock lock(mutex_);
  return IsEnabledUnSafe();
}

bool RasterThreadMerger::IsEnabledUnSafe() const {
  return enabled_;
}

bool RasterThreadMerger::IsMergedUnSafe() const {
  return TaskQueuesAreSame() || shared_merger_impl_->IsMergedUnSafe();
}

bool RasterThreadMerger::TaskQueuesAreSame() const {
  return platform_queue_id_ == gpu_queue_id_;
}

void RasterThreadMerger::WaitUntilMerged() {
  if (TaskQueuesAreSame()) {
    return;
  }
  FML_CHECK(IsOnPlatformThread());
  std::unique_lock<std::mutex> lock(mutex_);
  merged_condition_.wait(lock, [&] { return IsMergedUnSafe(); });
}

RasterThreadStatus RasterThreadMerger::DecrementLease() {
  if (TaskQueuesAreSame()) {
    return RasterThreadStatus::kRemainsMerged;
  }
  std::scoped_lock lock(mutex_);
  if (!IsMergedUnSafe()) {
    return RasterThreadStatus::kRemainsUnmerged;
  }
  if (!IsEnabledUnSafe()) {
    return RasterThreadStatus::kRemainsMerged;
  }
  bool unmerged_after_decrement = shared_merger_impl_->DecrementLease();
  if (unmerged_after_decrement) {
    if (merge_unmerge_callback_ != nullptr) {
      merge_unmerge_callback_();
    }
    return RasterThreadStatus::kUnmergedNow;
  }

  return RasterThreadStatus::kRemainsMerged;
}

void RasterThreadMerger::RecordMergeCaller() {
  std::scoped_lock lock(mutex_);
  shared_merger_impl_->RecordMergerCaller(this);
}

}  // namespace fml
