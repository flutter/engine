// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_handler_winuwp.h"

#include "third_party/cppwinrt/generated/winrt/Windows.ApplicationModel.DataTransfer.h"
#include "third_party/cppwinrt/generated/winrt/Windows.Foundation.h"
#include "third_party/cppwinrt/generated/winrt/Windows.UI.Core.h"

#include "flutter/shell/platform/windows/flutter_windows_view.h"
#include "flutter/shell/platform/windows/string_conversion.h"
namespace flutter {

// static
std::unique_ptr<PlatformHandler> PlatformHandler::Create(
    BinaryMessenger* messenger,
    FlutterWindowsView* view) {
  return std::make_unique<PlatformHandlerWinUwp>(messenger, view);
}

PlatformHandlerWinUwp::PlatformHandlerWinUwp(BinaryMessenger* messenger,
                                             FlutterWindowsView* view)
    : PlatformHandler(messenger), view_(view) {}

PlatformHandlerWinUwp::~PlatformHandlerWinUwp() = default;

void PlatformHandlerWinUwp::GetPlainText(
    std::unique_ptr<MethodResult<rapidjson::Document>> result,
    std::string_view key) {
  auto activation_mode =
      winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread()
          .ActivationMode();
  // We call `Clipboard::GetContent()` when the application window is in
  // focus, otherwise calling this will throw an error.
  if (activation_mode == winrt::Windows::UI::Core::CoreWindowActivationMode::
                             ActivatedInForeground) {
    auto content =
        winrt::Windows::ApplicationModel::DataTransfer::Clipboard::GetContent();
    // Calling `DataPackageView::GetTextAsync()` when the clipboard has no text
    // content will throw an error.
    if (content.Contains(winrt::Windows::ApplicationModel::DataTransfer::
                             StandardDataFormats::Text())) {
      // Waiting `DataPackageView::GetTextAsync()` using `TResult.get()` on the
      // platform thread will causes the application stop, so we continue
      // response on this callback.
      content.GetTextAsync().Completed([result = std::move(result), key](
                                           auto async_info,
                                           auto _async_status) {
        auto clipboard_string = async_info.GetResults();

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        document.AddMember(
            rapidjson::Value(key.data(), allocator),
            rapidjson::Value(Utf8FromUtf16(clipboard_string), allocator),
            allocator);
        result->Success(document);
      });
    } else {
      result->Success(rapidjson::Document());
    }
  } else {
    result->Success(rapidjson::Document());
  }
}

void PlatformHandlerWinUwp::GetHasStrings(
    std::unique_ptr<MethodResult<rapidjson::Document>> result) {
  // TODO: Implement. See https://github.com/flutter/flutter/issues/70214.
  result->NotImplemented();
}

void PlatformHandlerWinUwp::SetPlainText(
    const std::string& text,
    std::unique_ptr<MethodResult<rapidjson::Document>> result) {
  winrt::Windows::ApplicationModel::DataTransfer::DataPackage content;
  content.SetText(Utf16FromUtf8(text));
  winrt::Windows::ApplicationModel::DataTransfer::Clipboard::SetContent(
      content);

  result->Success();
}

}  // namespace flutter
