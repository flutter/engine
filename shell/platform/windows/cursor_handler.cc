// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/cursor_handler.h"

#include <windows.h>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

static constexpr char kChannelName[] = "flutter/mousecursor";

static constexpr char kActivateSystemCursorMethod[] = "activateSystemCursor";
static constexpr char kKindKey[] = "kind";

// This method allows setting a custom cursor with rawBGRA buffer.
static constexpr char kSetCustomCursorMethod[] = "setCustomCursor/windows";
// A list of bytes, the custom cursor's rawBGRA buffer.
static constexpr char kCustomCursorBufferKey[] = "buffer";
// A double, the x coordinate of the custom cursor's hotspot, starting from
// left.
static constexpr char kCustomCursorHotXKey[] = "hotX";
// A double, the y coordinate of the custom cursor's hotspot, starting from top.
static constexpr char kCustomCursorHotYKey[] = "hotY";
// int value for the width of custom cursor.
static constexpr char kCustomCursorWidthKey[] = "width";
// int value for the height of custom cursor.
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
  } else if (method.compare(kSetCustomCursorMethod) == 0) {
    const auto& arguments = std::get<EncodableMap>(*method_call.arguments());
    auto buffer_iter =
        arguments.find(EncodableValue(std::string(kCustomCursorBufferKey)));
    if (buffer_iter == arguments.end()) {
      result->Error(
          "Argument error",
          "Missing argument buffer while trying to customize system cursor");
      return;
    }
    auto buffer = std::get<std::vector<uint8_t>>(buffer_iter->second);
    auto width_iter =
        arguments.find(EncodableValue(std::string(kCustomCursorWidthKey)));
    if (width_iter == arguments.end()) {
      result->Error(
          "Argument error",
          "Missing argument width while trying to customize system cursor");
      return;
    }
    auto width = std::get<int>(arguments.at(width_iter->second));
    auto height_iter =
        arguments.find(EncodableValue(std::string(kCustomCursorHeightKey)));
    if (height_iter == arguments.end()) {
      result->Error(
          "Argument error",
          "Missing argument height while trying to customize system cursor");
      return;
    }
    auto height = std::get<int>(height_iter->second);
    auto hot_x_iter =
        arguments.find(EncodableValue(std::string(kCustomCursorHotXKey)));
    if (hot_x_iter == arguments.end()) {
      result->Error(
          "Argument error",
          "Missing argument hotX while trying to customize system cursor");
      return;
    }
    auto hot_x = std::get<double>(hot_x_iter->second);
    auto hot_y_iter =
        arguments.find(EncodableValue(std::string(kCustomCursorHotYKey)));
    if (hot_y_iter == arguments.end()) {
      result->Error(
          "Argument error",
          "Missing argument hotY while trying to customize system cursor");
      return;
    }
    auto hot_y = std::get<double>(hot_y_iter->second);
    HCURSOR cursor = GetCursorFromBuffer(buffer, hot_x, hot_y, width, height);
    if (cursor == nullptr) {
      result->Error("Argument error",
                    "Argument must contain valid rawBGRA bitmap");
      return;
    }
    delegate_->SetFlutterCursor(cursor);
    result->Success();
  } else {
    result->NotImplemented();
  }
}

HCURSOR GetCursorFromBuffer(const std::vector<uint8_t>& buffer,
                            double hot_x,
                            double hot_y,
                            int width,
                            int height) {
  HCURSOR cursor = nullptr;
  // Flutter returns rawRGRA, which has 8bits * 4channels.
  auto bitmap = CreateBitmap(width, height, 1, 32, &buffer[0]);
  if (bitmap == nullptr) {
    return nullptr;
  }
  ICONINFO icon_info;
  icon_info.fIcon = 0;
  icon_info.xHotspot = hot_x;
  icon_info.yHotspot = hot_y;
  icon_info.hbmMask = bitmap;
  icon_info.hbmColor = bitmap;
  cursor = CreateIconIndirect(&icon_info);
  DeleteObject(bitmap);
  return cursor;
}

}  // namespace flutter
