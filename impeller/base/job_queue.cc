// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/base/job_queue.h"

#include "flutter/fml/trace_event.h"
#include "impeller/base/validation.h"

namespace impeller {

JobQueue::JobQueue(std::shared_ptr<fml::ConcurrentTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {}

JobQueue::~JobQueue() = default;

std::shared_ptr<JobQueue> JobQueue::Make(
    std::shared_ptr<fml::ConcurrentTaskRunner> task_runner) {
  return std::shared_ptr<JobQueue>(new JobQueue(std::move(task_runner)));
}

UniqueID JobQueue::AddJob(fml::closure job) {
  auto id = UniqueID{};
  {
    Lock lock(jobs_mutex_);
    jobs_[id] = job;
  }
  ScheduleAndRunJob(id);
  return id;
}

void JobQueue::ScheduleAndRunJob(UniqueID id) {
  task_runner_->PostTask([weak = weak_from_this(), id]() {
    if (auto thiz = weak.lock()) {
      thiz->RunJobNow(id);
    }
  });
}

void JobQueue::PrioritizeJob(UniqueID id) {
  Lock lock(jobs_mutex_);
  high_priority_job_ids_.push_back(id);
}

void JobQueue::RunJobNow(UniqueID id) {
  while (RunHighPriorityJob()) {
  }
  fml::closure job;
  {
    Lock lock(jobs_mutex_);
    job = TakeJob(id);
  }
  if (job) {
    TRACE_EVENT0("impeller", "RegularJob");
    job();
  }
}

bool JobQueue::RunHighPriorityJob() {
  fml::closure job;
  {
    Lock lock(jobs_mutex_);
    if (high_priority_job_ids_.empty()) {
      return false;
    }
    auto job_id = high_priority_job_ids_.front();
    high_priority_job_ids_.pop_front();
    job = TakeJob(job_id);
  }
  if (job) {
    TRACE_EVENT0("impeller", "HighPriorityJob");
    job();
  }
  return true;
}

void JobQueue::DoJobNow(UniqueID id) {
  fml::closure job;
  {
    Lock lock(jobs_mutex_);
    job = TakeJob(id);
  }
  if (job) {
    TRACE_EVENT0("impeller", "EagerJob");
    job();
  }
}

fml::closure JobQueue::TakeJob(UniqueID id) {
  auto found = jobs_.find(id);
  if (found == jobs_.end()) {
    return nullptr;
  }
  auto job = found->second;
  jobs_.erase(found);
  return job;
}

const std::shared_ptr<fml::ConcurrentTaskRunner>& JobQueue::GetTaskRunner()
    const {
  return task_runner_;
}

}  // namespace impeller
