// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/base_jni_onload.h"
#include "base/android/base_jni_registrar.h"
#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "base/android/library_loader/library_loader_hooks.h"
#include "base/bind.h"
#include "flutter/fml/platform/android/jni_util.h"
#include "flutter/lib/jni/dart_jni.h"
#include "flutter/shell/platform/android/flutter_main.h"
#include "flutter/shell/platform/android/platform_view_android.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"
#include "lib/ftl/arraysize.h"

namespace {

bool RegisterJNI(JNIEnv* env) {
  if (!base::android::RegisterJni(env))
    return false;
  return true;
}

bool InitJNI() {
  return blink::DartJni::InitJni();
}

}  // namespace

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  // Initialize the Java VM.
  fml::jni::InitJavaVM(vm);

  JNIEnv* env = fml::jni::AttachCurrentThread();
  bool result = false;

  // Register FlutterMain.
  result = shell::RegisterFlutterMain(env);
  FTL_CHECK(result);

  // Register PlatformView
  result = shell::PlatformViewAndroid::Register(env);
  FTL_CHECK(result);

  // Register VSyncWaiter.
  result = shell::VsyncWaiterAndroid::Register(env);
  FTL_CHECK(result);

  std::vector<base::android::RegisterCallback> register_callbacks;
  register_callbacks.push_back(base::Bind(&RegisterJNI));

  std::vector<base::android::InitCallback> init_callbacks;
  init_callbacks.push_back(base::Bind(&InitJNI));

  if (!base::android::OnJNIOnLoadRegisterJNI(vm, register_callbacks) ||
      !base::android::OnJNIOnLoadInit(init_callbacks)) {
    return -1;
  }

  return JNI_VERSION_1_4;
}
