// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_view_manager.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/platform_views";
}

PlatformViewManager::PlatformViewManager(BinaryMessenger* binary_messenger)
    : channel_(std::make_unique<MethodChannel<EncodableValue>>(
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
          if (AddPlatformView(id, type)) {
            result->Success();
          } else {
            result->Error("AddPlatformView", "Failed to add platform view");
          }
          return;
        } else if (call.method_name() == "focus") {
          const auto& id =
              std::get<std::int32_t>(args.find(EncodableValue("id"))->second);
          const auto& direction = std::get<std::int32_t>(
              args.find(EncodableValue("direction"))->second);
          const auto& focus =
              std::get<bool>(args.find(EncodableValue("focus"))->second);
          if (FocusPlatformView(
                  id, static_cast<FocusChangeDirection>(direction), focus)) {
            result->Success();
          } else {
            result->Error("FocusPlatformView", "Failed to focus platform view");
          }
          return;
        }
        result->NotImplemented();
      });
}

PlatformViewManager::~PlatformViewManager() {}

}  // namespace flutter
