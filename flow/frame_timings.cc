// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is FrameTimingsRecorder::Governed by a BSD-style
// license that can be found in the LICENSE file.

#include "flutter/flow/frame_timings.h"
#include <mutex>
#include "flutter/common/settings.h"
#include "flutter/fml/logging.h"

namespace flutter {

FrameTimingsRecorder::FrameTimingsRecorder() = default;

FrameTimingsRecorder::~FrameTimingsRecorder() = default;

fml::TimePoint FrameTimingsRecorder::GetVsyncStartTime() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kVsync);
  return vsync_start_;
}

fml::TimePoint FrameTimingsRecorder::GetVsyncTargetTime() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kVsync);
  return vsync_target_;
}

fml::TimePoint FrameTimingsRecorder::GetBuildStartTime() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kBuildStart);
  return build_start_;
}

fml::TimePoint FrameTimingsRecorder::GetBuildEndTime() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kBuildEnd);
  return build_end_;
}

fml::TimePoint FrameTimingsRecorder::GetRasterStartTime() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kRasterStart);
  return raster_start_;
}

fml::TimePoint FrameTimingsRecorder::GetRasterEndTime() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kRasterEnd);
  return raster_end_;
}

fml::TimeDelta FrameTimingsRecorder::GetBuildDuration() const {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ >= State::kBuildEnd);
  return build_end_ - build_start_;
}

void FrameTimingsRecorder::RecordVsync(fml::TimePoint vsync_start,
                                       fml::TimePoint vsync_target) {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ == State::kUnitialized);
  state_ = State::kVsync;
  vsync_start_ = vsync_start;
  vsync_target_ = vsync_target;
}

void FrameTimingsRecorder::RecordBuildStart(fml::TimePoint build_start) {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ == State::kVsync);
  state_ = State::kBuildStart;
  build_start_ = build_start;
}

void FrameTimingsRecorder::RecordBuildEnd(fml::TimePoint build_end) {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ == State::kBuildStart);
  state_ = State::kBuildEnd;
  build_end_ = build_end;
}

void FrameTimingsRecorder::RecordRasterStart(fml::TimePoint raster_start) {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ == State::kBuildEnd);
  state_ = State::kRasterStart;
  raster_start_ = raster_start;
}

FrameTiming FrameTimingsRecorder::RecordRasterEnd(fml::TimePoint raster_end) {
  std::scoped_lock state_lock(state_mutex_);
  FML_CHECK(state_ == State::kRasterStart);
  state_ = State::kRasterEnd;
  raster_end_ = raster_end;
  FrameTiming timing;
  timing.Set(FrameTiming::kVsyncStart, vsync_start_);
  timing.Set(FrameTiming::kBuildStart, build_start_);
  timing.Set(FrameTiming::kBuildFinish, build_end_);
  timing.Set(FrameTiming::kRasterStart, raster_start_);
  timing.Set(FrameTiming::kRasterFinish, raster_end_);
  return timing;
}

}  // namespace flutter
