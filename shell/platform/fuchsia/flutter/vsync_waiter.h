// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_VSYNC_WAITER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_VSYNC_WAITER_H_

#include <lib/async/cpp/wait.h>

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "flutter_runner_product_configuration.h"

#include "session_connection.h"

namespace flutter_runner {

struct FlutterFrameTimes {
  fml::TimePoint frame_start;
  fml::TimePoint frame_target;
};

class VsyncWaiter final : public flutter::VsyncWaiter {
 public:
  static FlutterFrameTimes GetTargetTimes(fml::TimeDelta vsync_offset,
                                          fml::TimeDelta vsync_interval,
                                          fml::TimePoint last_targetted_vsync,
                                          fml::TimePoint now,
                                          fml::TimePoint next_vsync);

  static fml::TimePoint SnapToNextPhase(
      const fml::TimePoint now,
      const fml::TimePoint last_frame_presentation_time,
      const fml::TimeDelta presentation_interval);

  VsyncWaiter(std::shared_ptr<SessionConnection> session_connection,
              flutter::TaskRunners task_runners,
              fml::TimeDelta vsync_offset);

  ~VsyncWaiter() override;

 private:
  // |flutter::VsyncWaiter|
  void AwaitVSync() override;

  // |flutter::VsyncWaiter|
  void AwaitVSyncForSecondaryCallback() override;

  void FireCallbackMaybe();
  void OnVsync();

  FlutterFrameTimes GetTargetTimesHelper(bool secondary_callback);

  std::shared_ptr<SessionConnection> session_connection_;

  // This is the offset from vsync that Flutter should wake up at to begin work
  // on the next frame.
  fml::TimeDelta vsync_offset_;

  // This is the last Vsync we submitted as the frame_target_time to
  // FireCallback(). This value should be strictly increasing in order to
  // guarantee that animation code that relies on target vsyncs works correctly,
  // and that Flutter is not producing multiple frames in a small interval.
  fml::TimePoint last_targetted_vsync_;

  // This is true iff AwaitVSync() was called but we could not schedule a frame.
  bool frame_request_pending_ = false;

  std::unique_ptr<fml::WeakPtrFactory<VsyncWaiter>> weak_factory_ui_;
  fml::WeakPtrFactory<VsyncWaiter> weak_factory_;

  FML_DISALLOW_COPY_AND_ASSIGN(VsyncWaiter);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_VSYNC_WAITER_H_
