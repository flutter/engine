// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_VOLATILE_PATH_TRACKER_H_
#define FLUTTER_LIB_VOLATILE_PATH_TRACKER_H_

#include <unordered_set>

#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"
#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkPath.h"

namespace flutter {

/// A cache for paths drawn from dart:ui.
///
/// Whenever a flutter::CanvasPath is created, it must Insert an entry into
/// this cache. Whenever a frame is drawn, the shell must call OnFrame. The
/// cache will flip the volatility bit on the SkPath and remove it from the
/// cache. If the Dart object is released, Erase must be called to avoid
/// tracking a path that is no longer referenced in Dart code.
class VolatilePathTracker {
 public:
  struct Path {
    bool tracking_volatility = false;
    int frame_count = 0;
    SkPath path_;
  };

  explicit VolatilePathTracker(fml::RefPtr<fml::TaskRunner> ui_task_runner);

  static constexpr int number_of_frames_until_non_volatile = 2;

  // Starts tracking a path.
  // Must be called from the UI task runner.
  void Insert(std::shared_ptr<Path> path);

  // Removes a path from tracking.
  //
  // May be called from any thread.
  void Erase(std::shared_ptr<Path> path);

  // Called by the shell at the end of a frame after notifying Dart about idle
  // time.
  //
  // This method will flip the volatility bit to false for any paths that have
  // survived the |number_of_frames_until_non_volatile|.
  //
  // Must be called from the UI task runner.
  void OnFrame();

 private:
  fml::RefPtr<fml::TaskRunner> ui_task_runner_;
  std::unordered_set<std::shared_ptr<Path>> paths_;
  FML_DISALLOW_COPY_AND_ASSIGN(VolatilePathTracker);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_VOLATILE_PATH_TRACKER_H_
