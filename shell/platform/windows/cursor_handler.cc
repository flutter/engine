// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/cursor_handler.h"

#include <windows.h>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

static constexpr char kChannelName[] = "flutter/mousecursor";

static constexpr char kActivateSystemCursorMethod[] = "activateSystemCursor";
static constexpr char kSetSystemCursorMethod[] = "setSystemCursor";

static constexpr char kKindKey[] = "kind";
static constexpr char kCustomCursorBufferKey[] = "buffer";
static constexpr char kCustomCursorHotxKey[] = "hotx";
static constexpr char kCustomCursorHotyKey[] = "hoty";
static constexpr char kCustomCursorWidthKey[] = "width";
static constexpr char kCustomCursorHeightKey[] = "height";

namespace flutter {

CursorHandler::CursorHandler(BinaryMessenger* messenger,
                             WindowBindingHandler* delegate)
    : channel_(std::make_unique<MethodChannel<EncodableValue>>(
          messenger,
          kChannelName,
          &StandardMethodCodec::GetInstance())),
      delegate_(delegate) {
  channel_->SetMethodCallHandler(
      [this](const MethodCall<EncodableValue>& call,
             std::unique_ptr<MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

void CursorHandler::HandleMethodCall(
    const MethodCall<EncodableValue>& method_call,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  const std::string& method = method_call.method_name();
  if (method.compare(kActivateSystemCursorMethod) == 0) {
    const auto& arguments = std::get<EncodableMap>(*method_call.arguments());
    auto kind_iter = arguments.find(EncodableValue(std::string(kKindKey)));
    if (kind_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument while trying to activate system cursor");
      return;
    }
    const auto& kind = std::get<std::string>(kind_iter->second);
    delegate_->UpdateFlutterCursor(kind);
    result->Success();
  } else if (method.compare(kSetSystemCursorMethod) == 0) {
    const auto& arguments = std::get<EncodableMap>(*method_call.arguments());
    auto buffer_iter = arguments.find(EncodableValue(std::string(kCustomCursorBufferKey)));
    if (buffer_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument buffer while trying to customize system cursor");
      return;
    }
    auto buffer = std::get<std::vector<uint8_t>>(
        arguments.at(flutter::EncodableValue("buffer")));
    auto width_iter = arguments.find(EncodableValue(std::string(kCustomCursorWidthKey)));
    if (width_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument width while trying to customize system cursor");
      return;
    }
    auto width = std::get<int>(arguments.at(flutter::EncodableValue("width")));
    auto height_iter = arguments.find(EncodableValue(std::string(kCustomCursorHeightKey)));
    if (height_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument height while trying to customize system cursor");
      return;
    }
    auto height = std::get<int>(arguments.at(flutter::EncodableValue("height")));
    auto hotx_iter = arguments.find(EncodableValue(std::string(kCustomCursorHotxKey)));
    if (hotx_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument hotx while trying to customize system cursor");
      return;
    }
    auto hotx = std::get<double>(arguments.at(flutter::EncodableValue("hotx")));
    auto hoty_iter = arguments.find(EncodableValue(std::string(kCustomCursorHotyKey)));
    if (hoty_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument hoty while trying to customize system cursor");
      return;
    }
    auto hoty = std::get<double>(arguments.at(flutter::EncodableValue("hoty")));
    HCURSOR cursor = nullptr;
    // Flutter returns rawRgba, which has 8bits*4channels
    auto bitmap = CreateBitmap(width, height, 1, 32, &buffer[0]);
    if (bitmap == nullptr) {
      result->Error("Argument error", "Argument buffer must contain valid rawRgba bitmap");
      return;
    }
    ICONINFO icon_info;
    icon_info.fIcon = 0;
    icon_info.xHotspot = hotx;
    icon_info.yHotspot = hoty;
    icon_info.hbmMask = bitmap;
    icon_info.hbmColor = bitmap;
    cursor = CreateIconIndirect(&icon_info);
    DeleteObject(bitmap);
    if (cursor == nullptr) {
      result->Error("Argument error", "Create Icon failed, argument buffer must contain valid rawRgba bitmap");
      return;
    }
    delegate_->SetFlutterCursor(cursor);
    result->Success();
  } else {
    result->NotImplemented();
  }
}

}  // namespace flutter
