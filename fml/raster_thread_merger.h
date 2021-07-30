// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FML_SHELL_COMMON_TASK_RUNNER_MERGER_H_
#define FML_SHELL_COMMON_TASK_RUNNER_MERGER_H_

#include <condition_variable>
#include <mutex>

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/message_loop_task_queues.h"
#include "flutter/fml/shared_thread_merger.h"

namespace fml {

class MessageLoopImpl;

enum class RasterThreadStatus {
  kRemainsMerged,
  kRemainsUnmerged,
  kUnmergedNow
};

class RasterThreadMerger
    : public fml::RefCountedThreadSafe<RasterThreadMerger> {
 public:
  // Merges the raster thread into platform thread for the duration of
  // the lease term. Lease is managed by the caller by either calling
  // |ExtendLeaseTo| or |DecrementLease|.
  // When the caller merges with a lease term of say 2. The threads
  // are going to remain merged until 2 invocations of |DecreaseLease|,
  // unless an |ExtendLeaseTo| gets called.
  //
  // If the task queues are the same, we consider them statically merged.
  // When task queues are statically merged this method becomes no-op.
  void MergeWithLease(int lease_term);

  // Un-merges the threads now if current caller is the last merge caller,
  // and it resets the lease term to 0, otherwise it will remove the caller
  // record and return. The multiple caller records were recorded by the
  // |RecordMergeCaller| method.
  //
  // Must be executed on the raster task runner.
  //
  // If the task queues are the same, we consider them statically merged.
  // When task queues are statically merged, we never unmerge them and
  // this method becomes no-op.
  void UnMergeNowIfLastOne();

  // If the task queues are the same, we consider them statically merged.
  // When task queues are statically merged this method becomes no-op.
  void ExtendLeaseTo(int lease_term);

  // Returns |RasterThreadStatus::kUnmergedNow| if this call resulted in
  // splitting the raster and platform threads. Reduces the lease term by 1.
  //
  // If the task queues are the same, we consider them statically merged.
  // When task queues are statically merged this method becomes no-op.
  RasterThreadStatus DecrementLease();

  // Record current merge caller in the set of SharedThreadMerger object.
  // This method should be called before multiple merge callers of same
  // owner/subsumed pair are going to call |MergeWithLease| method.
  //
  // And the reason why not putting this method inside |MergeWithLease| is
  // |MergeWithLease| should not called when another caller already merged the
  // threads and |IsMerged| return true. In this case only recording the
  // current caller is needed.
  void RecordMergeCaller();

  // The method is locked by current instance, and asks the shared instance of
  // SharedThreadMerger and the merging state is determined by the
  // lease_term_ counter.
  bool IsMerged();

  // Waits until the threads are merged.
  //
  // Must run on the platform task runner.
  void WaitUntilMerged();

  RasterThreadMerger(fml::TaskQueueId platform_queue_id,
                     fml::TaskQueueId gpu_queue_id);

  // Returns true if the current thread owns rasterizing.
  // When the threads are merged, platform thread owns rasterizing.
  // When un-merged, raster thread owns rasterizing.
  bool IsOnRasterizingThread() const;

  // Returns true if the current thread is the platform thread.
  bool IsOnPlatformThread() const;

  // Enables the thread merger.
  void Enable();

  // Disables the thread merger. Once disabled, any call to
  // |MergeWithLease| or |UnMergeNowIfLastOne| results in a noop.
  void Disable();

  // Whether the thread merger is enabled. By default, the thread merger is
  // enabled. If false, calls to |MergeWithLease| or |UnMergeNowIfLastOne|
  // results in a noop.
  bool IsEnabled();

  // Registers a callback that can be used to clean up global state right after
  // the thread configuration has changed.
  //
  // For example, it can be used to clear the GL context so it can be used in
  // the next task from a different thread.
  void SetMergeUnmergeCallback(const fml::closure& callback);

 private:
  fml::TaskQueueId platform_queue_id_;
  fml::TaskQueueId gpu_queue_id_;
  SharedThreadMerger* shared_merger_;
  std::condition_variable merged_condition_;
  std::mutex mutex_;
  fml::closure merge_unmerge_callback_;
  bool enabled_;

  bool IsMergedUnSafe() const;

  bool IsEnabledUnSafe() const;

  // The platform_queue_id and gpu_queue_id are exactly the same.
  // We consider the threads are always merged and cannot be unmerged.
  bool TaskQueuesAreSame() const;

  FML_FRIEND_REF_COUNTED_THREAD_SAFE(RasterThreadMerger);
  FML_FRIEND_MAKE_REF_COUNTED(RasterThreadMerger);
  FML_DISALLOW_COPY_AND_ASSIGN(RasterThreadMerger);
};

}  // namespace fml

#endif  // FML_SHELL_COMMON_TASK_RUNNER_MERGER_H_
