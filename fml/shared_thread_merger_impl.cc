// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/shared_thread_merger_impl.h"

#include <set>

namespace fml {

std::mutex SharedThreadMergerImpl::creation_mutex_;

// Guarded by creation_mutex_
std::map<ThreadMergerKey, fml::RefPtr<SharedThreadMergerImpl>>
    SharedThreadMergerImpl::shared_merger_instances_;

const int SharedThreadMergerImpl::kLeaseNotSet = -1;

fml::RefPtr<SharedThreadMergerImpl> SharedThreadMergerImpl::GetSharedImpl(
    fml::TaskQueueId owner,
    fml::TaskQueueId subsumed) {
  std::scoped_lock creation(creation_mutex_);
  ThreadMergerKey key = {.owner = owner, .subsumed = subsumed};
  auto iter = shared_merger_instances_.find(key);
  if (iter != shared_merger_instances_.end()) {
    return iter->second;
  }
  auto merger = fml::MakeRefCounted<SharedThreadMergerImpl>(owner, subsumed);
  shared_merger_instances_[key] = merger;
  return merger;
}

SharedThreadMergerImpl::SharedThreadMergerImpl(fml::TaskQueueId owner,
                                               fml::TaskQueueId subsumed)
    : owner_(owner),
      subsumed_(subsumed),
      task_queues_(fml::MessageLoopTaskQueues::GetInstance()),
      lease_term_(kLeaseNotSet) {}

void SharedThreadMergerImpl::RecordMergerCaller(RasterThreadMerger* caller) {
  std::scoped_lock lock(mutex_);
  // Record current merge caller into callers set.
  merge_callers_.insert(caller);
}

bool SharedThreadMergerImpl::MergeWithLease(int lease_term) {
  FML_DCHECK(lease_term > 0) << "lease_term should be positive.";
  std::scoped_lock lock(mutex_);
  if (IsMergedUnSafe()) {
    return true;
  }
  bool success = task_queues_->Merge(owner_, subsumed_);
  FML_CHECK(success) << "Unable to merge the raster and platform threads.";

  lease_term_ = lease_term;
  return success;
}

bool SharedThreadMergerImpl::UnMergeNowUnSafe() {
  FML_CHECK(lease_term_ == 0)
      << "lease_term_ must be 0 state before calling UnMergeNowUnSafe()";
  bool success = task_queues_->Unmerge(owner_, subsumed_);
  FML_CHECK(success) << "Unable to un-merge the raster and platform threads.";
  return success;
}

bool SharedThreadMergerImpl::UnMergeNowIfLastOne(RasterThreadMerger* caller) {
  std::scoped_lock lock(mutex_);
  merge_callers_.erase(caller);
  if (!merge_callers_.empty()) {
    return true;
  }
  FML_CHECK(IsMergedUnSafe())
      << "UnMergeNowIfLastOne() must be called only when threads are merged";
  lease_term_ = 0;  // mark need unmerge now
  return UnMergeNowUnSafe();
}

bool SharedThreadMergerImpl::DecrementLease() {
  std::scoped_lock lock(mutex_);
  FML_DCHECK(lease_term_ > 0)
      << "lease_term should always be positive when merged.";
  lease_term_--;
  if (lease_term_ == 0) {
    // Unmerge now because lease_term_ decreased to zero.
    UnMergeNowUnSafe();
    return true;
  }
  return false;
}

void SharedThreadMergerImpl::ExtendLeaseTo(int lease_term) {
  FML_DCHECK(lease_term > 0) << "lease_term should be positive.";
  std::scoped_lock lock(mutex_);
  FML_DCHECK(IsMergedUnSafe())
      << "should be merged state when calling this method";
  if (lease_term_ != kLeaseNotSet && lease_term > lease_term_) {
    lease_term_ = lease_term;
  }
}

bool SharedThreadMergerImpl::IsMergedUnSafe() const {
  return lease_term_ > 0;
}

}  // namespace fml
