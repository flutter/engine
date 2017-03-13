// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/android/jni_util.h"
#include "lib/ftl/logging.h"

namespace fml {
namespace jni {

static JavaVM* g_jvm = nullptr;
static ScopedJavaGlobalRef<jobject>* g_android_application_context = nullptr;

JNIEnv* AttachCurrentThread() {
  FTL_DCHECK(g_jvm != nullptr)
      << "Trying to attach to current thread without calling InitJavaVM first.";
  JNIEnv* env = nullptr;
  jint ret = g_jvm->AttachCurrentThread(&env, nullptr);
  FTL_DCHECK(JNI_OK == ret);
  return env;
}

void InitJavaVM(JavaVM* vm) {
  FTL_DCHECK(g_jvm == nullptr);
  g_jvm = vm;
}

void InitAndroidApplicationContext(const JavaRef<jobject>& context) {
  FTL_DCHECK(g_android_application_context == nullptr);
  g_android_application_context = new ScopedJavaGlobalRef<jobject>(context);
  FTL_DCHECK(g_android_application_context->obj() != nullptr);
}

const jobject GetAndroidApplicationContext() {
  jobject object = g_android_application_context->obj();
  FTL_DCHECK(object != nullptr)
      << "Trying to get Android application context without first calling "
         "InitAndroidApplicationContext.";
  return object;
}

}  // namespace jni
}  // namespace fml
