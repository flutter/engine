// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/cursor_handler.h"

#include <windows.h>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_method_codec.h"
#include "flutter/shell/platform/windows/win32_flutter_window.h"

static constexpr char kChannelName[] = "flutter/mousecursor";

static constexpr char kActivateSystemCursorMethod[] = "activateSystemCursor";

static constexpr char kKindKey[] = "kind";

namespace flutter {

namespace {

// Maps a Flutter cursor constant to an HCURSOR.
//
// Returns the arrow cursor for unknown constants.
static HCURSOR GetCursorForKind(const std::string& kind) {
  // The following mapping must be kept in sync with Flutter framework's
  // mouse_cursor.dart
  if (kind.compare("none") == 0) {
    return nullptr;
  }
  const wchar_t* cursor_name = IDC_ARROW;
  if (kind.compare("basic") == 0) {
    cursor_name = IDC_ARROW;
  } else if (kind.compare("click") == 0) {
    cursor_name = IDC_HAND;
  } else if (kind.compare("text") == 0) {
    cursor_name = IDC_IBEAM;
  } else if (kind.compare("forbidden") == 0) {
    cursor_name = IDC_NO;
  } else if (kind.compare("horizontalDoubleArrow") == 0) {
    cursor_name = IDC_SIZEWE;
  } else if (kind.compare("verticalDoubleArrow") == 0) {
    cursor_name = IDC_SIZENS;
  }
  return ::LoadCursor(nullptr, cursor_name);
}

}  // namespace

CursorHandler::CursorHandler(flutter::BinaryMessenger* messenger,
                             Win32FlutterWindow* window)
    : channel_(std::make_unique<flutter::MethodChannel<EncodableValue>>(
          messenger,
          kChannelName,
          &flutter::StandardMethodCodec::GetInstance())),
      window_(window) {
  channel_->SetMethodCallHandler(
      [this](const flutter::MethodCall<EncodableValue>& call,
             std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

void CursorHandler::HandleMethodCall(
    const flutter::MethodCall<EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  const std::string& method = method_call.method_name();
  if (method.compare(kActivateSystemCursorMethod) == 0) {
    const flutter::EncodableMap& arguments =
        method_call.arguments()->MapValue();
    auto kind_iter = arguments.find(EncodableValue(kKindKey));
    if (kind_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument while trying to activate system cursor");
    }
    const std::string& kind = kind_iter->second.StringValue();
    window_->UpdateFlutterCursor(GetCursorForKind(kind));
    result->Success();
  } else {
    result->NotImplemented();
  }
}

}  // namespace flutter
