// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/volatile_path_tracker.h"

namespace flutter {

VolatilePathTracker::VolatilePathTracker(
    fml::RefPtr<fml::TaskRunner> ui_task_runner)
    : ui_task_runner_(ui_task_runner) {}

void VolatilePathTracker::Insert(std::shared_ptr<Path> path) {
  FML_DCHECK(ui_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(path);
  FML_DCHECK(path->path.isVolatile());
  paths_.insert(path);
}

void VolatilePathTracker::Erase(std::shared_ptr<Path> path) {
  FML_DCHECK(path);
  if (ui_task_runner_->RunsTasksOnCurrentThread()) {
    paths_.erase(path);
    return;
  }

  needs_drain_ = true;
  std::scoped_lock lock(paths_to_remove_mutex_);
  paths_to_remove_.push_back(path);
}

void VolatilePathTracker::OnFrame() {
  FML_DCHECK(ui_task_runner_->RunsTasksOnCurrentThread());
  TRACE_EVENT1("flutter", "VolatilePathTracker::OnFrame", "count",
               std::to_string(paths_.size()).c_str());

  Drain();

  for (auto it = paths_.begin(), last = paths_.end(); it != last;) {
    auto path = *it;
    path->frame_count++;
    if (path->frame_count >= kFramesOfVolatility) {
      path->path.setIsVolatile(false);
      path->tracking_volatility = false;
      it = paths_.erase(it);
    } else {
      ++it;
    }
  }
  TRACE_EVENT_INSTANT1("flutter", "VolatilePathTracker::OnFrame", "count",
                       std::to_string(paths_.size()).c_str());
}

void VolatilePathTracker::Drain() {
  if (needs_drain_) {
    TRACE_EVENT0("flutter", "VolatilePathTracker::Drain");
    std::deque<std::shared_ptr<Path>> paths_to_remove;
    {
      std::scoped_lock lock(paths_to_remove_mutex_);
      paths_to_remove.swap(paths_to_remove_);
    }
    TRACE_EVENT_INSTANT1("flutter", "VolatilePathTracker::Drain", "count",
                         std::to_string(paths_to_remove.size()).c_str());
    for (auto path : paths_to_remove) {
      paths_.erase(path);
    }
    needs_drain_ = false;
  }
}

}  // namespace flutter
