// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_FRAME_TIMINGS_H_
#define FLUTTER_FLOW_FRAME_TIMINGS_H_

#include <mutex>

#include "flutter/common/settings.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"

namespace flutter {

class FrameTimingsRecorder {
 public:
  FrameTimingsRecorder();

  ~FrameTimingsRecorder();

  fml::TimePoint GetVsyncStartTime() const;

  fml::TimePoint GetVsyncTargetTime() const;

  fml::TimePoint GetBuildStartTime() const;

  fml::TimePoint GetBuildEndTime() const;

  fml::TimePoint GetRasterStartTime() const;

  fml::TimePoint GetRasterEndTime() const;

  fml::TimeDelta GetBuildDuration() const;

  void RecordVsync(fml::TimePoint vsync_start, fml::TimePoint vsync_target);

  void RecordBuildStart(fml::TimePoint build_start);

  void RecordBuildEnd(fml::TimePoint build_end);

  void RecordRasterStart(fml::TimePoint raster_start);

  FrameTiming RecordRasterEnd(fml::TimePoint raster_end);

 private:
  enum class State : uint32_t {
    kUnitialized = 1,
    kVsync = 2,
    kBuildStart = 3,
    kBuildEnd = 4,
    kRasterStart = 5,
    kRasterEnd = 6,
  };

  mutable std::mutex state_mutex_;
  State state_ = State::kUnitialized;

  fml::TimePoint vsync_start_ = fml::TimePoint::Min();
  fml::TimePoint vsync_target_ = fml::TimePoint::Min();
  fml::TimePoint build_start_ = fml::TimePoint::Min();
  fml::TimePoint build_end_ = fml::TimePoint::Min();
  fml::TimePoint raster_start_ = fml::TimePoint::Min();
  fml::TimePoint raster_end_ = fml::TimePoint::Min();

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(FrameTimingsRecorder);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_FRAME_TIMINGS_H_
