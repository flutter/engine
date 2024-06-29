// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_MESSAGE_HANDLER_ANDROID_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_MESSAGE_HANDLER_ANDROID_H_

#include <jni.h>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/lib/ui/window/platform_message_listener_dart_port.h"
#include "flutter/shell/common/platform_message_handler.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"

namespace flutter {
class PlatformMessageHandlerAndroid : public PlatformMessageHandler {
 public:
  explicit PlatformMessageHandlerAndroid(
      const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade);
  void HandlePlatformMessage(std::unique_ptr<PlatformMessage> message) override;
  bool DoesHandlePlatformMessageOnPlatformThread() const override {
    return false;
  }
  void InvokePlatformMessageResponseCallback(
      int response_id,
      std::unique_ptr<fml::Mapping> mapping) override;

  void InvokePlatformMessageEmptyResponseCallback(int response_id) override;

  bool AddPlatformPortCallback(int64_t port,
                               const std::string& channel) override;
  bool RemovePlatformPortCallback(const std::string& channel) override;
  bool SendToPlatformPortCallback(
      const std::string& name,
      std::unique_ptr<fml::MallocMapping> message_data,
      int32_t response_id,
      fml::RefPtr<flutter::PlatformMessageResponse> response);
  bool SendToPlatformPortCallbackEmpty(
      const std::string& name,
      int32_t response_id,
      fml::RefPtr<flutter::PlatformMessageResponse> response);

  void SendToPlatformPortCallbackResponse(
      int response_id,
      std::unique_ptr<fml::Mapping> mapping) override;
  void SendToPlatformPortCallbackResponseEmpty(int response_id) override;

 private:
  const std::shared_ptr<PlatformViewAndroidJNI> jni_facade_;
  std::atomic<int> next_response_id_ = 1;
  std::unordered_map<std::string,
                     fml::RefPtr<flutter::PlatformListenerDartPort>>
      active_callbacks_;
  std::unordered_map<int, fml::RefPtr<flutter::PlatformMessageResponse>>
      pending_callback_responses_;
  std::unordered_map<int, fml::RefPtr<flutter::PlatformMessageResponse>>
      pending_responses_;
  std::mutex pending_responses_mutex_;
  std::mutex pending_callback_responses_mutex_;
};
}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_MESSAGE_HANDLER_ANDROID_H_
