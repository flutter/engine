// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_MOCK_PLATFORM_VIEW_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_MOCK_PLATFORM_VIEW_MANAGER_H_

#include "flutter/shell/platform/windows/platform_view_manager.h"

#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "gmock/gmock.h"

namespace flutter {

class MockPlatformViewManager : public PlatformViewManager {
 public:
  MockPlatformViewManager(FlutterWindowsEngine* engine) : PlatformViewManager(engine->task_runner(), engine->messenger_wrapper()) {}

  ~MockPlatformViewManager() {}

  MOCK_METHOD(void, QueuePlatformViewCreation, (std::string_view, int64_t id));
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_TESTING_MOCK_PLATFORM_VIEW_MANAGER_H_
