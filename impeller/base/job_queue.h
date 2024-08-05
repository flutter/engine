// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_BASE_JOB_QUEUE_H_
#define FLUTTER_IMPELLER_BASE_JOB_QUEUE_H_

#include <deque>
#include <map>
#include <memory>

#include "flutter/fml/closure.h"
#include "flutter/fml/concurrent_message_loop.h"
#include "impeller/base/comparable.h"
#include "impeller/base/thread.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      Manages a queue of jobs that execute on a concurrent task
///             runner. Jobs execute as soon as resources are available. Callers
///             have the ability to re-prioritize jobs or perform jobs eagerly
///             on their own threads if needed.
///
///             The job queue and all its methods are thread-safe.
///
class JobQueue final : public std::enable_shared_from_this<JobQueue> {
 public:
  //----------------------------------------------------------------------------
  /// @brief      Creates a job queue which schedules tasks on the given
  ///             concurrent task runner.
  ///
  /// @param[in]  task_runner  The task runner
  ///
  /// @return     A job queue if one can be created.
  ///
  static std::shared_ptr<JobQueue> Make(
      std::shared_ptr<fml::ConcurrentTaskRunner> task_runner);

  //----------------------------------------------------------------------------
  /// @brief      Destroys the job queue.
  ///
  ~JobQueue();

  JobQueue(const JobQueue&) = delete;

  JobQueue& operator=(const JobQueue&) = delete;

  //----------------------------------------------------------------------------
  /// @brief      Adds a job to the job queue and schedules it for execution at
  ///             a later point in time. The job ID obtained may be used to
  ///             re-prioritize the job within the queue at a later point.
  ///
  /// @param[in]  job   The job
  ///
  /// @return     The unique id for the job.
  ///
  UniqueID AddJob(fml::closure job);

  //----------------------------------------------------------------------------
  /// @brief      Prioritize a previously added job for immediate execution on
  ///             the concurrent task runner.
  ///
  /// @param[in]  id    The job identifier.
  ///
  void PrioritizeJob(UniqueID id);

  //----------------------------------------------------------------------------
  /// @brief      If the job has not already been completed, executes the job
  ///             immediately on the callers thread.
  ///
  ///             This is useful if the current thread is going to idle anyway
  ///             and wants to participate in performing the job of the
  ///             concurrent task runner.
  ///
  /// @param[in]  id    The job identifier.
  ///
  void DoJobNow(UniqueID id);

  //----------------------------------------------------------------------------
  /// @brief      Gets the task runner for the queue.
  ///
  /// @return     The task runner.
  ///
  const std::shared_ptr<fml::ConcurrentTaskRunner>& GetTaskRunner() const;

 private:
  std::shared_ptr<fml::ConcurrentTaskRunner> task_runner_;
  Mutex jobs_mutex_;
  std::map<UniqueID, fml::closure> jobs_ IPLR_GUARDED_BY(jobs_mutex_);
  std::deque<UniqueID> high_priority_job_ids_ IPLR_GUARDED_BY(jobs_mutex_);

  JobQueue(std::shared_ptr<fml::ConcurrentTaskRunner> task_runner);

  void ScheduleAndRunJob(UniqueID id);

  void RunJobNow(UniqueID id);

  bool RunHighPriorityJob();

  [[nodiscard]]
  fml::closure TakeJob(UniqueID id) IPLR_REQUIRES(jobs_mutex_);
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_BASE_JOB_QUEUE_H_
