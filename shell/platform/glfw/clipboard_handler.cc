// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/clipboard_handler.h"

#include <iostream>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/json_method_codec.h"

static constexpr char kChannelName[] = "flutter/platform";

static constexpr char kGetClipboardDataMethod[] = "Clipboard.getData";
static constexpr char kSetClipboardDataMethod[] = "Clipboard.setData";

static constexpr char kTextPlainFormat[] = "text/plain";

static constexpr char kUnknownClipboardFormatError[] =
    "Unknown clipboard formaterror";
namespace flutter {

ClipboardHandler::ClipboardHandler(
    flutter::BinaryMessenger* messenger,
    FlutterDesktopWindowRef window,
    GetClipboardDataCallback get_clipboard_callback,
    SetClipboardDataCallback set_clipboard_callback)
    : channel_(std::make_unique<flutter::MethodChannel<rapidjson::Document>>(
          messenger,
          kChannelName,
          &flutter::JsonMethodCodec::GetInstance())),
      window_(window),
      get_clipboard_callback_(get_clipboard_callback),
      set_clipboard_callback_(set_clipboard_callback) {
  channel_->SetMethodCallHandler(
      [this](
          const flutter::MethodCall<rapidjson::Document>& call,
          std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

ClipboardHandler::~ClipboardHandler() = default;

void ClipboardHandler::HandleMethodCall(
    const flutter::MethodCall<rapidjson::Document>& method_call,
    std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
  const char* kTextKey = "text";

  const std::string& method = method_call.method_name();

  if (method.compare(kGetClipboardDataMethod) == 0) {
    // Only one string argument is expected.
    const rapidjson::Value& format = method_call.arguments()[0];

    if (strcmp(format.GetString(), kTextPlainFormat) != 0) {
      result->Error(kUnknownClipboardFormatError,
                    "GLFW clipboard API only supports text.");
      return;
    }

    const char* clipboardData = get_clipboard_callback_(window_);

    if (clipboardData == NULL) {
      result->Error(kUnknownClipboardFormatError,
                    "Failed to retrieve clipboard data from GLFW api.");
      return;
    }
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember(rapidjson::Value(kTextKey, allocator),
                       rapidjson::Value(clipboardData, allocator), allocator);
    result->Success(&document);
    return;
  } else if (method.compare(kSetClipboardDataMethod) == 0) {
    const rapidjson::Value& format = *method_call.arguments();
    rapidjson::Value::ConstMemberIterator itr = format.FindMember(kTextKey);
    const char* string = itr->value.GetString();
    set_clipboard_callback_(window_, string);
  } else {
    result->NotImplemented();
    return;
  }
  result->Success();
}
}  // namespace flutter