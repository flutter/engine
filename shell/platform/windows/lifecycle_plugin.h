// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_LIFECYCLE_PLUGIN_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_LIFECYCLE_PLUGIN_H_

#include "flutter/shell/platform/common/client_wrapper/include/flutter/basic_message_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"

namespace flutter {

// Implements a lifecycle plugin.
class LifecyclePlugin {
 public:
  explicit LifecyclePlugin(flutter::BinaryMessenger* messenger);

  // Send to engine that the app is resumed.
  void SendAppIsResumed();

  // Send to engine that the app is inactive.
  void SendAppIsInactive();

  // Send to engine that the app is paused.
  void SendAppIsPaused();

  // Send to engine that the app is detached.
  void SendAppIsDetached();

 private:
  // The MethodChannel used for communication with the Flutter engine.
  std::unique_ptr<flutter::BasicMessageChannel<std::string>> channel_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_LIFECYCLE_PLUGIN_H_
