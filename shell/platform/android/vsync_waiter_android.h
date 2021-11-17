// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_ANDROID_VSYNC_WAITER_ANDROID_H_
#define SHELL_PLATFORM_ANDROID_VSYNC_WAITER_ANDROID_H_

#include <jni.h>

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/vsync_waiter.h"

namespace flutter {

struct AChoreographer;
typedef struct AChoreographer AChoreographer;

/**
 * Prototype of the function that is called when a new frame is being rendered.
 * It's passed the time that the frame is being rendered as nanoseconds in the
 * CLOCK_MONOTONIC time base, as well as the data pointer provided by the
 * application that registered a callback. All callbacks that run as part of
 * rendering a frame will observe the same frame time, so it should be used
 * whenever events need to be synchronized (e.g. animations).
 */
typedef void (*AChoreographer_frameCallback)(long frameTimeNanos, void* data);

// AChoreographer is supported from API 24. To allow compilation for minSDK < 24
// and still use AChoreographer for SDK >= 24 we need runtime support to call
// AChoreographer APIs.
using PFN_AChoreographer_getInstance = AChoreographer* (*)();

using PFN_AChoreographer_postFrameCallback =
    void (*)(AChoreographer* choreographer,
             AChoreographer_frameCallback callback,
             void* data);

class VsyncWaiterAndroid final : public VsyncWaiter {
 public:
  static bool Register(JNIEnv* env);

  VsyncWaiterAndroid(flutter::TaskRunners task_runners);

  ~VsyncWaiterAndroid() override;

 private:
  // |VsyncWaiter|
  void AwaitVSync() override;

  static void OnNativeVsync(JNIEnv* env,
                            jclass jcaller,
                            jlong frameDelayNanos,
                            jlong refreshPeriodNanos,
                            jlong java_baton);

  static void OnAChoreographerVsync(long frameTimeNanos, void* java_baton);

  static void SetRefreshRateFPS(JNIEnv* env,
                                jobject jcaller,
                                jfloat refreshRateFPS);

  static void ConsumePendingCallback(jlong java_baton,
                                     fml::TimePoint frame_start_time,
                                     fml::TimePoint frame_target_time);

  FML_DISALLOW_COPY_AND_ASSIGN(VsyncWaiterAndroid);

 private:
  PFN_AChoreographer_getInstance mAChoreographer_getInstance = nullptr;
  PFN_AChoreographer_postFrameCallback mAChoreographer_postFrameCallback =
      nullptr;
  bool useAChoreographer_ = false;
};

}  // namespace flutter

#endif  // SHELL_PLATFORM_ANDROID_ASYNC_WAITER_ANDROID_H_
