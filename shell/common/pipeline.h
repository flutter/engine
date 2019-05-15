// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_PIPELINE_H_
#define FLUTTER_SHELL_COMMON_PIPELINE_H_

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/synchronization/semaphore.h"
#include "flutter/fml/trace_event.h"

namespace flutter {

enum class PipelineConsumeResult {
  NoneAvailable,
  Done,
  MoreAvailable,
};

enum class PipelineItemStateChange {
  // Indicates that a new pipeline item has started.
  kBegin,
  // Indicates that the pipeline item has advanced to its next phase.  In
  // practice this will only happen once, and means that the pipeline item has
  // finished its UI thread work, and that GPU thread work is about to begin.
  kStep,
  // Indicates that the pipeline item is done.
  kEnd,
  // Indicates that the pipeline item was dropped.
  kAbort,
};

// Called when pipeline items change states.  See
// |Settings::pipeline_state_observer| for further details.
using PipelineStateObserver =
    std::function<void(intptr_t pipeline_id,
                       size_t pipeline_item,
                       PipelineItemStateChange state_change)>;

size_t GetNextPipelineTraceID();

template <class R>
class Pipeline : public fml::RefCountedThreadSafe<Pipeline<R>> {
 public:
  using Resource = R;
  using ResourcePtr = std::unique_ptr<Resource>;

  /// Denotes a spot in the pipeline reserved for the producer to finish
  /// preparing a completed pipeline resource.
  class ProducerContinuation {
   public:
    ProducerContinuation() : trace_id_(0) {}

    ProducerContinuation(ProducerContinuation&& other)
        : continuation_(other.continuation_), trace_id_(other.trace_id_) {
      other.continuation_ = nullptr;
      other.trace_id_ = 0;
    }

    ProducerContinuation& operator=(ProducerContinuation&& other) {
      std::swap(continuation_, other.continuation_);
      std::swap(trace_id_, other.trace_id_);
      return *this;
    }

    ~ProducerContinuation() {
      if (continuation_) {
        continuation_(nullptr, trace_id_);
        TRACE_EVENT_ASYNC_END0("flutter", "PipelineProduce", trace_id_);
        // The continuation is being dropped on the floor. End the flow.
        TRACE_FLOW_END("flutter", "PipelineItem", trace_id_);
        TRACE_EVENT_ASYNC_END0("flutter", "PipelineItem", trace_id_);
      }
    }

    void Complete(ResourcePtr resource) {
      if (continuation_) {
        continuation_(std::move(resource), trace_id_);
        continuation_ = nullptr;
        TRACE_EVENT_ASYNC_END0("flutter", "PipelineProduce", trace_id_);
        TRACE_FLOW_STEP("flutter", "PipelineItem", trace_id_);
      }
    }

    operator bool() const { return continuation_ != nullptr; }

   private:
    friend class Pipeline;
    using Continuation = std::function<void(ResourcePtr, size_t)>;

    Continuation continuation_;
    size_t trace_id_;

    ProducerContinuation(Continuation continuation, size_t trace_id)
        : continuation_(continuation), trace_id_(trace_id) {
      TRACE_FLOW_BEGIN("flutter", "PipelineItem", trace_id_);
      TRACE_EVENT_ASYNC_BEGIN0("flutter", "PipelineItem", trace_id_);
      TRACE_EVENT_ASYNC_BEGIN0("flutter", "PipelineProduce", trace_id_);
    }

    FML_DISALLOW_COPY_AND_ASSIGN(ProducerContinuation);
  };

  Pipeline(uint32_t depth, PipelineStateObserver pipeline_state_observer)
      : empty_(depth),
        available_(0),
        pipeline_state_observer_(std::move(pipeline_state_observer)) {}

  ~Pipeline() = default;

  bool IsValid() const { return empty_.IsValid() && available_.IsValid(); }

  ProducerContinuation Produce() {
    if (!empty_.TryWait()) {
      return {};
    }

    size_t trace_id = GetNextPipelineTraceID();
    if (pipeline_state_observer_) {
      pipeline_state_observer_(PipelineId(), trace_id,
                               PipelineItemStateChange::kBegin);
    }

    return ProducerContinuation{
        std::bind(&Pipeline::ProducerCommit, this, std::placeholders::_1,
                  std::placeholders::_2),  // continuation
        trace_id                           // trace id
    };
  }

  using Consumer = std::function<void(ResourcePtr)>;

  FML_WARN_UNUSED_RESULT
  PipelineConsumeResult Consume(Consumer consumer) {
    if (consumer == nullptr) {
      return PipelineConsumeResult::NoneAvailable;
    }

    if (!available_.TryWait()) {
      return PipelineConsumeResult::NoneAvailable;
    }

    ResourcePtr resource;
    size_t trace_id = 0;
    size_t items_count = 0;

    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      std::tie(resource, trace_id) = std::move(queue_.front());
      queue_.pop();
      items_count = queue_.size();
    }

    {
      TRACE_EVENT0("flutter", "PipelineConsume");
      consumer(std::move(resource));
    }

    empty_.Signal();

    TRACE_FLOW_END("flutter", "PipelineItem", trace_id);
    TRACE_EVENT_ASYNC_END0("flutter", "PipelineItem", trace_id);
    if (pipeline_state_observer_) {
      pipeline_state_observer_(PipelineId(), trace_id,
                               PipelineItemStateChange::kEnd);
    }
    return items_count > 0 ? PipelineConsumeResult::MoreAvailable
                           : PipelineConsumeResult::Done;
  }

 private:
  fml::Semaphore empty_;
  fml::Semaphore available_;
  std::mutex queue_mutex_;
  std::queue<std::pair<ResourcePtr, size_t>> queue_;
  PipelineStateObserver pipeline_state_observer_;

  void ProducerCommit(ResourcePtr resource, size_t trace_id) {
    bool had_resource = !!resource;

    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      queue_.emplace(std::move(resource), trace_id);
    }

    // Ensure the queue mutex is not held as that would be a pessimization.
    available_.Signal();

    if (pipeline_state_observer_) {
      if (had_resource) {
        pipeline_state_observer_(PipelineId(), trace_id,
                                 PipelineItemStateChange::kStep);
      } else {
        pipeline_state_observer_(PipelineId(), trace_id,
                                 PipelineItemStateChange::kAbort);
      }
    }
  }

  intptr_t PipelineId() const { return intptr_t(this); }

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(Pipeline);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_PIPELINE_H_
