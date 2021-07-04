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

struct ThreadMergerKey {
  TaskQueueId owner;
  TaskQueueId subsumed;
  bool operator<(const ThreadMergerKey &other) const {
    if (owner == other.owner) {
      return subsumed < other.subsumed;
    } else {
      return owner < other.owner;
    }
  }
};

class SharedThreadMergerImpl : public fml::RefCountedThreadSafe<SharedThreadMergerImpl> {
 public:
  SharedThreadMergerImpl(fml::TaskQueueId owner, fml::TaskQueueId subsumed);
  static fml::RefPtr<SharedThreadMergerImpl> GetSharedImpl(fml::TaskQueueId owner, fml::TaskQueueId subsumed);
  void RecordMergerCaller(RasterThreadMerger *caller);
  bool MergeWithLease(size_t lease_term);
  /// TODO DOC
  bool UnMergeNowIfLastOne(RasterThreadMerger *caller);
  void ExtendLeaseTo(size_t lease_term);
  bool IsMergedUnSafe() const;
  bool DecrementLease();
 private:
  static const int kLeaseNotSet;
  fml::TaskQueueId owner_;
  fml::TaskQueueId subsumed_;
  fml::RefPtr<fml::MessageLoopTaskQueues> task_queues_;
  std::atomic_int lease_term_;
  std::mutex mutex_;
  std::set<fml::RasterThreadMerger *> merge_callers_;
  static std::mutex creation_mutex_;
  // Guarded by creation_mutex_
  static std::map<ThreadMergerKey, fml::RefPtr<SharedThreadMergerImpl>> shared_merger_instances_;
  bool UnMergeNowUnSafe();
};

}  // namespace fml

#endif  // FLUTTER_FML_SHARED_THREAD_MERGER_H_
