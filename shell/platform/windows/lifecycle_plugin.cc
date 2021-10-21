// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/lifecycle_plugin.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/string_message_codec.h"

namespace flutter {

namespace {

constexpr char kChannelName[] = "flutter/lifecycle";

constexpr char kStateResumed[] = "AppLifecycleState.resumed";
constexpr char kStateInactive[] = "AppLifecycleState.inactive";
constexpr char kStatePaused[] = "AppLifecycleState.paused";
constexpr char kStateDetached[] = "AppLifecycleState.detached";

}  // namespace

LifecyclePlugin::LifecyclePlugin(BinaryMessenger* messenger)
    : channel_(std::make_unique<flutter::BasicMessageChannel<std::string>>(
          messenger,
          kChannelName,
          &StringMessageCodec::GetInstance())) {}

void LifecyclePlugin::SendAppIsResumed() {
  channel_->Send(kStateResumed);
}

void LifecyclePlugin::SendAppIsInactive() {
  channel_->Send(kStateInactive);
}

void LifecyclePlugin::SendAppIsPaused() {
  channel_->Send(kStatePaused);
}

void LifecyclePlugin::SendAppIsDetached() {
  channel_->Send(kStateDetached);
}

}  // namespace flutter
