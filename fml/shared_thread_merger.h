// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_SHARED_THREAD_MERGER_H_
#define FLUTTER_FML_SHARED_THREAD_MERGER_H_

#include <condition_variable>
#include <mutex>

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/message_loop_task_queues.h"

namespace fml {

class MessageLoopImpl;
class RasterThreadMerger;

typedef void* RasterThreadMergerId;

class SharedThreadMerger
    : public fml::RefCountedThreadSafe<SharedThreadMerger> {
 public:
  SharedThreadMerger(TaskQueueId owner, TaskQueueId subsumed);

  // It's called by |RasterThreadMerger::RecordMergerCaller()|.
  // See the doc of |RasterThreadMerger::RecordMergerCaller()|.
  void RecordMergerCaller(RasterThreadMergerId caller);

  // It's called by |RasterThreadMerger::MergeWithLease()|.
  // See the doc of |RasterThreadMerger::MergeWithLease()|.
  bool MergeWithLease(int lease_term);

  // It's called by |RasterThreadMerger::UnMergeNowIfLastOne()|.
  // See the doc of |RasterThreadMerger::UnMergeNowIfLastOne()|.
  bool UnMergeNowIfLastOne(RasterThreadMergerId caller);

  // It's called by |RasterThreadMerger::ExtendLeaseTo()|.
  // See the doc of |RasterThreadMerger::ExtendLeaseTo()|.
  void ExtendLeaseTo(int lease_term);

  // It's called by |RasterThreadMerger::IsMergedUnSafe()|.
  // See the doc of |RasterThreadMerger::IsMergedUnSafe()|.
  bool IsMergedUnSafe() const;

  // It's called by |RasterThreadMerger::DecrementLease()|.
  // See the doc of |RasterThreadMerger::DecrementLease()|.
  bool DecrementLease();

 private:
  static const int kLeaseNotSet;
  fml::TaskQueueId owner_;
  fml::TaskQueueId subsumed_;
  fml::RefPtr<fml::MessageLoopTaskQueues> task_queues_;
  std::atomic_int lease_term_;
  std::mutex mutex_;

  /// The |RecordMergerCaller| method will record the caller
  /// into this merge_callers_ set, |UnMergeNowIfLastOne()|
  /// method will remove the caller from this merge_callers_.
  std::set<RasterThreadMergerId> merge_callers_;

  bool UnMergeNowUnSafe();

  FML_DISALLOW_COPY_AND_ASSIGN(SharedThreadMerger);
};

}  // namespace fml

#endif  // FLUTTER_FML_SHARED_THREAD_MERGER_H_
