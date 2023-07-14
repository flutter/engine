// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_VSYNC_WAITER_ANDROID_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_VSYNC_WAITER_ANDROID_H_

#include <android/choreographer.h>
#include <jni.h>

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "impeller/toolkit/android/choreographer.h"

namespace flutter {

class VsyncWaiterAndroid final : public VsyncWaiter {
 public:
  static bool Register(JNIEnv* env);

  explicit VsyncWaiterAndroid(const flutter::TaskRunners& task_runners);

  ~VsyncWaiterAndroid() override;

 private:
  // |VsyncWaiter|
  void AwaitVSync() override;

  void Loop();

  static fml::TimePoint NormalizeFrameTime(fml::TimePoint frame_time);

  static void OnVsyncFromNDK(int64_t vsync_nanos, void* data);
  // This needs to match a deprecated NDK interface.
  static void OnVsyncFromNDK32(long frame_nanos,  // NOLINT
                               void* data);

  static void OnVsyncFromNDK33(
      impeller::android::ChoreographerVsyncTimings timings,
      void* data);

  static void OnVsyncFromJava(JNIEnv* env,
                              jclass jcaller,
                              jlong frameDelayNanos,
                              jlong refreshPeriodNanos,
                              jlong java_baton);

  static void ConsumePendingCallback(
      std::weak_ptr<VsyncWaiterAndroid>* weak_this,
      FrameTimingsRecorder::VSyncInfo vsync_info);

  static void OnUpdateRefreshRate(JNIEnv* env,
                                  jclass jcaller,
                                  jfloat refresh_rate);

  bool awaiting_ = false;
  bool looping_ = false;
  FML_DISALLOW_COPY_AND_ASSIGN(VsyncWaiterAndroid);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_VSYNC_WAITER_ANDROID_H_
