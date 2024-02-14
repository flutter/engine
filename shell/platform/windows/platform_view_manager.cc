// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_view_manager.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/platform_views";
}

PlatformViewManager::PlatformViewManager(TaskRunner* task_runner,
                                         BinaryMessenger* binary_messenger)
    : task_runner_(task_runner),
      channel_(std::make_unique<MethodChannel<EncodableValue>>(
          binary_messenger,
          kChannelName,
          &StandardMethodCodec::GetInstance())) {
  channel_->SetMethodCallHandler(
      [this](const MethodCall<EncodableValue>& call,
             std::unique_ptr<MethodResult<EncodableValue>> result) {
        const auto& args = std::get<EncodableMap>(*call.arguments());
        if (call.method_name() == "create") {
          const auto& type =
              std::get<std::string>(args.find(EncodableValue("type"))->second);
          const auto& id =
              std::get<std::int32_t>(args.find(EncodableValue("id"))->second);
          QueuePlatformViewCreation(id, type);
        }
        result->Success();
      });
}

PlatformViewManager::~PlatformViewManager() {}

void PlatformViewManager::QueuePlatformViewCreation(
    PlatformViewId id,
    std::string_view type_name) {}

void PlatformViewManager::InstantiatePlatformView(PlatformViewId id) {}

void PlatformViewManager::RegisterPlatformViewType(
    std::string_view type_name,
    const FlutterPlatformViewTypeEntry& type) {}

void PlatformViewManager::FocusPlatformView(PlatformViewId id,
                                            FocusChangeDirection direction,
                                            bool focus) {}

std::optional<HWND> PlatformViewManager::GetNativeHandleForId(
    PlatformViewId id) const {
  return std::nullopt;
}

}  // namespace flutter
