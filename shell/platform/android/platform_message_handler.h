// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_ANDROID_PLATFORM_MESSAGE_HANDLER_H_
#define SHELL_PLATFORM_ANDROID_PLATFORM_MESSAGE_HANDLER_H_

#include <jni.h>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"

namespace flutter {
class PlatformMessageHandler {
 public:
  PlatformMessageHandler(
      const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade);
  void HandlePlatformMessage(std::unique_ptr<PlatformMessage> message);
  void InvokePlatformMessageResponseCallback(JNIEnv* env,
                                             jint response_id,
                                             jobject java_response_data,
                                             jint java_response_position);

  void InvokePlatformMessageEmptyResponseCallback(JNIEnv* env,
                                                  jint response_id);

 private:
  const std::shared_ptr<PlatformViewAndroidJNI> jni_facade_;
  int next_response_id_ = 1;
  std::unordered_map<int, fml::RefPtr<flutter::PlatformMessageResponse>>
      pending_responses_;
  std::mutex pending_responses_mutex_;
};
}  // namespace flutter

#endif
