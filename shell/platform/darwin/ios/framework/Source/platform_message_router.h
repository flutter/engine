// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_PLATFORM_MESSAGE_ROUTER_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_PLATFORM_MESSAGE_ROUTER_H_

#include <unordered_map>

#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/platform/darwin/scoped_block.h"
#include "flutter/lib/ui/window/platform_message.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"

namespace flutter {

class PlatformMessageRouter {
 public:
  PlatformMessageRouter();
  ~PlatformMessageRouter();

  void HandlePlatformMessage(
      std::unique_ptr<flutter::PlatformMessage> message) const;

  std::unique_ptr<fml::Mapping> HandleFFIPlatformMessage(
      std::unique_ptr<flutter::PlatformMessage> message) const;

  void SetMessageHandler(const std::string& channel,
                         FlutterBinaryMessageHandler handler);

  void SetFFIMessageHandler(const std::string& channel,
                            FlutterFFIBinaryMessageHandler handler);

 private:
  mutable std::mutex ffi_mutex_;
  std::unordered_map<std::string, fml::ScopedBlock<FlutterBinaryMessageHandler>>
      message_handlers_;
  std::unordered_map<std::string,
                     fml::ScopedBlock<FlutterFFIBinaryMessageHandler>>
      ffi_message_handlers_;
  FML_DISALLOW_COPY_AND_ASSIGN(PlatformMessageRouter);
};

}  // namespace flutter

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_ACCESSIBILITY_BRIDGE_H_
