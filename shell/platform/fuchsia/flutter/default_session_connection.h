// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DEFAULT_SESSION_CONNECTION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DEFAULT_SESSION_CONNECTION_H_

#include <fuchsia/scenic/scheduling/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/ui/scenic/cpp/session.h>

#include "flutter/flow/scene_update_context.h"
#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"

#include "vsync_waiter.h"

#include <mutex>

namespace flutter_runner {

using on_frame_presented_event =
    std::function<void(fuchsia::scenic::scheduling::FramePresentedInfo)>;

struct FlutterFrameTimes {
  fml::TimePoint frame_start;
  fml::TimePoint frame_target;
};

// The component residing on the raster thread that is responsible for
// maintaining the Scenic session connection and presenting node updates.
class DefaultSessionConnection final : public flutter::SessionWrapper {
 public:
  static fml::TimePoint CalculateNextLatchPoint(
      fml::TimePoint present_requested_time,
      fml::TimePoint now,
      fml::TimePoint last_latch_point_targeted,
      fml::TimeDelta flutter_frame_build_time,
      fml::TimeDelta vsync_interval,
      std::deque<std::pair<fml::TimePoint, fml::TimePoint>>&
          future_presentation_infos);

  static FlutterFrameTimes GetTargetTimes(fml::TimeDelta vsync_offset,
                                          fml::TimeDelta vsync_interval,
                                          fml::TimePoint last_targetted_vsync,
                                          fml::TimePoint now,
                                          fml::TimePoint next_vsync);

  static fml::TimePoint SnapToNextPhase(
      const fml::TimePoint now,
      const fml::TimePoint last_frame_presentation_time,
      const fml::TimeDelta presentation_interval);

  DefaultSessionConnection(
      std::string debug_label,
      fidl::InterfaceHandle<fuchsia::ui::scenic::Session> session,
      fml::closure session_error_callback,
      on_frame_presented_event on_frame_presented_callback,
      uint64_t max_frames_in_flight,
      fml::TimeDelta vsync_offset);

  ~DefaultSessionConnection();

  // |SessionWrapper|
  scenic::Session* get() override { return &session_wrapper_; }

  // |SessionWrapper|
  void Present() override;

  // Used to implement VsyncWaiter functionality.
  void AwaitVsync(FireCallbackCallback callback);
  void AwaitVsyncForSecondaryCallback(FireCallbackCallback callback);

 private:
  scenic::Session session_wrapper_;

  on_frame_presented_event on_frame_presented_callback_;

  fml::TimePoint last_latch_point_targeted_ =
      fml::TimePoint::FromEpochDelta(fml::TimeDelta::Zero());
  fml::TimePoint present_requested_time_ =
      fml::TimePoint::FromEpochDelta(fml::TimeDelta::Zero());

  std::deque<std::pair<fml::TimePoint, fml::TimePoint>>
      future_presentation_infos_ = {};

  bool initialized_ = false;

  // A flow event trace id for following |Session::Present| calls into
  // Scenic.  This will be incremented each |Session::Present| call.  By
  // convention, the Scenic side will also contain its own trace id that
  // begins at 0, and is incremented each |Session::Present| call.
  uint64_t next_present_trace_id_ = 0;
  uint64_t next_present_session_trace_id_ = 0;
  uint64_t processed_present_session_trace_id_ = 0;

  // The maximum number of frames Flutter sent to Scenic that it can have
  // outstanding at any time. This is equivalent to how many times it has
  // called Present2() before receiving an OnFramePresented() event.
  const int kMaxFramesInFlight;
  fml::TimeDelta vsync_offset_;

  int frames_in_flight_ = 0;
  int frames_in_flight_allowed_ = 0;
  bool present_session_pending_ = false;

  ////// Flutter Animator logic.

  std::mutex mutex_;

  // This is the last Vsync we submitted as the frame_target_time to
  // FireCallback(). This value should be strictly increasing in order to
  // guarantee that animation code that relies on target vsyncs works correctly,
  // and that Flutter is not producing multiple frames in a small interval.
  fml::TimePoint last_targetted_vsync_;

  // This is true iff AwaitVSync() was called but we could not schedule a frame.
  bool fire_callback_request_pending_ = false;

  FireCallbackCallback fire_callback_;

  void PresentSession();

  void FireCallbackMaybe();
  FlutterFrameTimes GetTargetTimesHelper(bool secondary_callback);

  FML_DISALLOW_COPY_AND_ASSIGN(DefaultSessionConnection);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DEFAULT_SESSION_CONNECTION_H_
