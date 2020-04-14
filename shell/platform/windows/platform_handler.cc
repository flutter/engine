// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_handler.h"

#include <windows.h>

#include <iostream>

#include "flutter/shell/platform/common/cpp/json_method_codec.h"
#include "flutter/shell/platform/windows/string_conversion.h"
#include "flutter/shell/platform/windows/win32_flutter_window.h"

static constexpr char kChannelName[] = "flutter/platform";

static constexpr char kGetClipboardDataMethod[] = "Clipboard.getData";
static constexpr char kSetClipboardDataMethod[] = "Clipboard.setData";

static constexpr char kTextPlainFormat[] = "text/plain";
static constexpr char kTextKey[] = "text";

static constexpr char kClipboardError[] = "Clipboard error";
static constexpr char kUnknownClipboardFormatMessage[] =
    "Unknown clipboard format";

namespace flutter {

namespace {

// A scoped wrapper for GlobalLock/GlobalUnlock.
class ScopedGlobalLock {
 public:
  // Attempts to acquire a global lock on |memory| for the life of this object.
  ScopedGlobalLock(HGLOBAL memory) {
    source_ = memory;
    locked_memory_ = ::GlobalLock(memory);
  }

  ~ScopedGlobalLock() {
    if (locked_memory_) {
      ::GlobalUnlock(source_);
    }
  }

  // Returns the locked memory pointer, which will be nullptr if acquiring the
  // lock failed.
  void* get() { return locked_memory_; }

 private:
  HGLOBAL source_;
  void* locked_memory_;
};

// A Clipboard wrapper that automatically closes the clipboard when it goes out
// of scope.
class ScopedClipboard {
 public:
  ScopedClipboard();
  ~ScopedClipboard();

  // Attempts to open the clipboard for the given window, returning true if
  // successful.
  bool Open(HWND window);

  // Returns true if there is string data available to get.
  bool HasString();

  // Returns string data from the clipboard.
  //
  // If getting a string fails, returns false and does not change |out_string|.
  // Get error information with ::GetLastError().
  //
  // Open(...) must have succeeded to call this method.
  bool GetString(std::wstring* out_string);

  // Sets the string content of the clipboard, returning true on success.
  //
  // On failure, get error information with ::GetLastError().
  //
  // Open(...) must have succeeded to call this method.
  bool SetString(const std::wstring string);

 private:
  bool opened_ = false;
};

ScopedClipboard::ScopedClipboard() {}

ScopedClipboard::~ScopedClipboard() {
  if (opened_) {
    ::CloseClipboard();
  }
}

bool ScopedClipboard::Open(HWND window) {
  opened_ = ::OpenClipboard(window);
  return opened_;
}

bool ScopedClipboard::HasString() {
  // Allow either plain text format, since getting data will auto-interpolate.
  return ::IsClipboardFormatAvailable(CF_UNICODETEXT) ||
         ::IsClipboardFormatAvailable(CF_TEXT);
}

bool ScopedClipboard::GetString(std::wstring* out_string) {
  assert(out_string != nullptr);
  assert(opened_);

  HANDLE data = ::GetClipboardData(CF_UNICODETEXT);
  if (data == nullptr) {
    return false;
  }
  ScopedGlobalLock locked_data(data);
  if (!locked_data.get()) {
    return false;
  }
  *out_string = static_cast<wchar_t*>(locked_data.get());
  return true;
}

bool ScopedClipboard::SetString(const std::wstring string) {
  assert(opened_);
  if (!::EmptyClipboard()) {
    return false;
  }
  size_t null_terminated_byte_count = sizeof(wchar_t) * (string.size() + 1);
  ScopedGlobalLock destination_memory(
      ::GlobalAlloc(GMEM_MOVEABLE, null_terminated_byte_count));
  memcpy(destination_memory.get(), string.c_str(), null_terminated_byte_count);
  if (!::SetClipboardData(CF_UNICODETEXT, destination_memory.get())) {
    return false;
  }
  return true;
}

}  // namespace

PlatformHandler::PlatformHandler(flutter::BinaryMessenger* messenger,
                                 Win32FlutterWindow* window)
    : channel_(std::make_unique<flutter::MethodChannel<rapidjson::Document>>(
          messenger,
          kChannelName,
          &flutter::JsonMethodCodec::GetInstance())),
      window_(window) {
  channel_->SetMethodCallHandler(
      [this](
          const flutter::MethodCall<rapidjson::Document>& call,
          std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

void PlatformHandler::HandleMethodCall(
    const flutter::MethodCall<rapidjson::Document>& method_call,
    std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
  const std::string& method = method_call.method_name();
  if (method.compare(kGetClipboardDataMethod) == 0) {
    // Only one string argument is expected.
    const rapidjson::Value& format = method_call.arguments()[0];

    if (strcmp(format.GetString(), kTextPlainFormat) != 0) {
      result->Error(kClipboardError, kUnknownClipboardFormatMessage);
      return;
    }

    ScopedClipboard clipboard;
    if (!clipboard.Open(window_->GetWindowHandle())) {
      rapidjson::Document error_code;
      error_code.SetInt(::GetLastError());
      result->Error(kClipboardError, "Unable to open clipboard", &error_code);
      return;
    }
    if (!clipboard.HasString()) {
      rapidjson::Document null;
      result->Success(&null);
      return;
    }
    std::wstring clipboard_string;
    if (!clipboard.GetString(&clipboard_string)) {
      rapidjson::Document error_code;
      error_code.SetInt(::GetLastError());
      result->Error(kClipboardError, "Unable to get clipboard data",
                    &error_code);
      return;
    }

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember(
        rapidjson::Value(kTextKey, allocator),
        rapidjson::Value(Utf8FromUtf16(clipboard_string), allocator),
        allocator);
    result->Success(&document);
  } else if (method.compare(kSetClipboardDataMethod) == 0) {
    const rapidjson::Value& document = *method_call.arguments();
    rapidjson::Value::ConstMemberIterator itr = document.FindMember(kTextKey);
    if (itr == document.MemberEnd()) {
      result->Error(kClipboardError, kUnknownClipboardFormatMessage);
      return;
    }

    ScopedClipboard clipboard;
    if (!clipboard.Open(window_->GetWindowHandle())) {
      rapidjson::Document error_code;
      error_code.SetInt(::GetLastError());
      result->Error(kClipboardError, "Unable to open clipboard", &error_code);
      return;
    }
    if (!clipboard.SetString(Utf16FromUtf8(itr->value.GetString()))) {
      rapidjson::Document error_code;
      error_code.SetInt(::GetLastError());
      result->Error(kClipboardError, "Unable to set clipboard data",
                    &error_code);
      return;
    }
    result->Success();
  } else {
    result->NotImplemented();
  }
}

}  // namespace flutter
