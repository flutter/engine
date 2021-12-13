// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/volatile_path_tracker.h"

namespace flutter {

VolatilePathTracker::VolatilePathTracker(
    fml::RefPtr<fml::TaskRunner> ui_task_runner,
    bool enabled)
    : ui_task_runner_(ui_task_runner), enabled_(enabled) {}

void VolatilePathTracker::Insert(std::shared_ptr<TrackedPath> path) {
  FML_DCHECK(ui_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(path);
  FML_DCHECK(path->path.isVolatile());
  FML_DCHECK(!path->erased);
  if (!enabled_) {
    path->path.setIsVolatile(false);
    return;
  }
  paths_.push_back(path);
}

void VolatilePathTracker::Erase(std::shared_ptr<TrackedPath> path) {
  if (!enabled_) {
    return;
  }
  FML_DCHECK(path);
  std::scoped_lock lock(paths_mutex_);
  path->erased = true;
  path->path.reset();
}

void VolatilePathTracker::OnFrame() {
  FML_DCHECK(ui_task_runner_->RunsTasksOnCurrentThread());
  if (!enabled_) {
    return;
  }
#if !FLUTTER_RELEASE
  std::string total_count = std::to_string(paths_.size());
  TRACE_EVENT1("flutter", "VolatilePathTracker::OnFrame", "total_count",
               total_count.c_str());
#else
  TRACE_EVENT0("flutter", "VolatilePathTracker::OnFrame");
#endif

  std::scoped_lock lock(paths_mutex_);
  std::vector<std::shared_ptr<TrackedPath>> surviving_paths;
  for (const std::shared_ptr<TrackedPath>& path : paths_) {
    if (path->erased) {
      continue;
    }
    path->frame_count++;
    if (path->frame_count >= kFramesOfVolatility) {
      path->path.setIsVolatile(false);
      path->tracking_volatility = false;
    } else {
      surviving_paths.push_back(path);
    }
  }
  paths_ = std::move(surviving_paths);

#if !FLUTTER_RELEASE
  std::string post_removal_count = std::to_string(paths_.size());
  TRACE_EVENT_INSTANT1("flutter", "VolatilePathTracker::OnFrame",
                       "remaining_count", post_removal_count.c_str());
#endif
}

}  // namespace flutter
