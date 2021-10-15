// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_PLATFORM_MESSAGE_HANDLER_H_
#define SHELL_COMMON_PLATFORM_MESSAGE_HANDLER_H_

#include <memory>

#include "flutter/lib/ui/window/platform_message.h"

namespace flutter {
class PlatformMessageHandler {
 public:
  virtual ~PlatformMessageHandler() = default;
  virtual void HandlePlatformMessage(
      std::unique_ptr<PlatformMessage> message) = 0;
};
}  // namespace flutter

#endif  // SHELL_COMMON_PLATFORM_MESSAGE_HANDLER_H_
