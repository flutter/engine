// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_TESTS_FAKE_SESSION_CONNECTION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_TESTS_FAKE_SESSION_CONNECTION_H_

namespace flutter_runner_test {

#include "flutter/shell/platform/fuchsia/flutter/session_connection.h"

class FakeSessionConnection : public flutter_runner::SessionConnection {
 public:
  FakeSessionConnection() {}

  void InitializeVsyncWaiterCallback(
      flutter_runner::VsyncWaiterCallback callback) override {
    callback_ = callback;
  }

  bool CanRequestNewFrames() override { return can_request_new_frames_; }

  void FireVsyncWaiterCallback() { callback_(); }

  void SetCanRequestNewFrames(bool new_bool) {
    can_request_new_frames_ = new_bool;
  }

 private:
  bool can_request_new_frames_ = true;
  flutter_runner::VsyncWaiterCallback callback_;
};

}  // namespace flutter_runner_test

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_TESTS_FAKE_SESSION_CONNECTION_H_
