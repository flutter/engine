// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/vsync_waiter_android.h"

#include <cmath>
#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/native_library.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/fml/platform/android/scoped_java_ref.h"
#include "flutter/fml/size.h"
#include "flutter/fml/trace_event.h"

namespace flutter {

static fml::jni::ScopedJavaGlobalRef<jclass>* g_vsync_waiter_class = nullptr;
static jmethodID g_async_wait_for_vsync_method_ = nullptr;
static std::optional<int64_t> g_vsync_period_nanos_ = std::nullopt;

namespace {
// Only avialalbe on API 24+
typedef void AChoreographer;

// Must be called from the Platform thread.
AChoreographer* GetAChoreographer() {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto get_instance_fn = libandroid->ResolveFunction<AChoreographer* (*)(void)>(
      "AChoreographer_getInstance");
  if (get_instance_fn) {
    return get_instance_fn.value()();
  }
  return nullptr;
}

// Only available on API 30+
typedef void (*AChoreographer_refreshRateCallback)(int64_t vsyncPeriodNanos,
                                                   void* data);
void HandleRefreshRateChanged(int64_t vsyncPeriodNanos, void* data) {
  g_vsync_period_nanos_ = vsyncPeriodNanos;
}

// The data parameter will not be accessed, but will serve as an identifier for
// unregistering.
void ListenToRefreshRateChanges(void* data) {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto register_refresh_rate_callback_fn = libandroid->ResolveFunction<void (*)(
      AChoreographer*, AChoreographer_refreshRateCallback, void*)>(
      "AChoreographer_registerRefreshRateCallback");

  if (register_refresh_rate_callback_fn) {
    auto choreographer = GetAChoreographer();
    FML_CHECK(choreographer);
    register_refresh_rate_callback_fn.value()(choreographer,
                                              &HandleRefreshRateChanged, data);
  }
}

// The data parmaeter must be the same as the pointer passed to
// ListenToRefreshRateChanges.
void StopListeningToRefreshRateChanges(void* data) {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto unregister_refresh_rate_callback_fn =
      libandroid->ResolveFunction<void (*)(
          AChoreographer*, AChoreographer_refreshRateCallback, void*)>(
          "AChoreographer_unregisterRefreshRateCallback");

  if (unregister_refresh_rate_callback_fn) {
    auto choreographer = GetAChoreographer();
    FML_CHECK(choreographer);
    unregister_refresh_rate_callback_fn.value()(
        choreographer, &HandleRefreshRateChanged, data);
  }
}
}  // namespace

VsyncWaiterAndroid::VsyncWaiterAndroid(flutter::TaskRunners task_runners)
    : VsyncWaiter(std::move(task_runners)) {
  ListenToRefreshRateChanges(this);
}

VsyncWaiterAndroid::~VsyncWaiterAndroid() {
  StopListeningToRefreshRateChanges(this);
}

// |VsyncWaiter|
void VsyncWaiterAndroid::AwaitVSync() {
  auto* weak_this = new std::weak_ptr<VsyncWaiter>(shared_from_this());
  jlong java_baton = reinterpret_cast<jlong>(weak_this);

  task_runners_.GetPlatformTaskRunner()->PostTask([java_baton]() {
    JNIEnv* env = fml::jni::AttachCurrentThread();
    env->CallStaticVoidMethod(g_vsync_waiter_class->obj(),     //
                              g_async_wait_for_vsync_method_,  //
                              java_baton                       //
    );
  });
}

// static
void VsyncWaiterAndroid::OnNativeVsync(JNIEnv* env,
                                       jclass jcaller,
                                       jlong frameDelayNanos,
                                       jlong refreshPeriodNanos,
                                       jlong java_baton) {
  TRACE_EVENT0("flutter", "VSYNC");

  auto frame_time =
      fml::TimePoint::Now() - fml::TimeDelta::FromNanoseconds(frameDelayNanos);
  auto target_time =
      frame_time + fml::TimeDelta::FromNanoseconds(
                       g_vsync_period_nanos_.value_or(refreshPeriodNanos));

  ConsumePendingCallback(java_baton, frame_time, target_time);
}

// static
void VsyncWaiterAndroid::ConsumePendingCallback(
    jlong java_baton,
    fml::TimePoint frame_start_time,
    fml::TimePoint frame_target_time) {
  auto* weak_this = reinterpret_cast<std::weak_ptr<VsyncWaiter>*>(java_baton);
  auto shared_this = weak_this->lock();
  delete weak_this;

  if (shared_this) {
    shared_this->FireCallback(frame_start_time, frame_target_time);
  }
}

// static
bool VsyncWaiterAndroid::Register(JNIEnv* env) {
  static const JNINativeMethod methods[] = {{
      .name = "nativeOnVsync",
      .signature = "(JJJ)V",
      .fnPtr = reinterpret_cast<void*>(&OnNativeVsync),
  }};

  jclass clazz = env->FindClass("io/flutter/embedding/engine/FlutterJNI");

  if (clazz == nullptr) {
    return false;
  }

  g_vsync_waiter_class = new fml::jni::ScopedJavaGlobalRef<jclass>(env, clazz);

  FML_CHECK(!g_vsync_waiter_class->is_null());

  g_async_wait_for_vsync_method_ = env->GetStaticMethodID(
      g_vsync_waiter_class->obj(), "asyncWaitForVsync", "(J)V");

  FML_CHECK(g_async_wait_for_vsync_method_ != nullptr);

  return env->RegisterNatives(clazz, methods, fml::size(methods)) == 0;
}

}  // namespace flutter
