// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_EMBEDDER_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_EMBEDDER_HANDLER_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/keyboard_key_handler.h"

namespace flutter {

namespace {
// An arbitrary size for the character cache in bytes.
//
// It should hold a UTF-32 character encoded in UTF-8 as long as the trailing
// '\0'.
constexpr size_t kCharacterCacheSize = 8;
}  // namespace

// A delegate of |KeyboardKeyHandler| that handles events by sending
// converted |FlutterKeyEvent|s through the embedder API.
//
// This class corresponds to the HardwareKeyboard API in the framework.
class KeyboardKeyEmbedderHandler
    : public KeyboardKeyHandler::KeyboardKeyHandlerDelegate {
 public:
  using SendEvent = std::function<void(const FlutterKeyEvent& /* event */,
                                       FlutterKeyEventCallback /* callback */,
                                       void* /* user_data */)>;

  // Build a KeyboardKeyEmbedderHandler.
  //
  // Use `send_event` to define how the manager should dispatch converted
  // flutter events, as well as how to receive the resopnse, to the engine. It's
  // typically FlutterWindowsEngine::SendKeyEvent.
  explicit KeyboardKeyEmbedderHandler(SendEvent send_event);

  virtual ~KeyboardKeyEmbedderHandler();

  // |KeyboardHandlerBase|
  bool KeyboardHook(int key,
                    int scancode,
                    int action,
                    char32_t character,
                    bool extended,
                    bool was_down,
                    std::function<void(bool)> callback) override;

 private:
  struct PendingResponse {
    std::function<void(bool, uint64_t)> callback;
    uint64_t response_id;
  };

  void cacheUtf8String(char32_t ch);

  std::function<void(const FlutterKeyEvent&,
                     FlutterKeyEventCallback,
                     void*)> sendEvent_;
  std::map<uint64_t, uint64_t> pressingRecords_;
  char character_cache_[kCharacterCacheSize];
  std::map<uint64_t, std::unique_ptr<PendingResponse>> pending_responses_;
  uint64_t response_id_;

  static uint64_t getPhysicalKey(int scancode, bool extended);
  static uint64_t getLogicalKey(int key, bool extended, int scancode);

  static std::map<uint64_t, uint64_t> windowsToPhysicalMap_;
  static std::map<uint64_t, uint64_t> windowsToLogicalMap_;
  static std::map<uint64_t, uint64_t> scanCodeToLogicalMap_;

  static void HandleResponse(bool handled, void* user_data);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_KEYBOARD_KEY_EMBEDDER_HANDLER_H_
