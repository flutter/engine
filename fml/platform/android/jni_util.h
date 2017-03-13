// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_ANDROID_JNI_UTIL_H_
#define FLUTTER_FML_PLATFORM_ANDROID_JNI_UTIL_H_

#include <jni.h>
#include "flutter/fml/platform/android/scoped_java_ref.h"
#include "lib/ftl/macros.h"

namespace fml {
namespace jni {

JNIEnv* AttachCurrentThread();

void InitJavaVM(JavaVM* vm);

void InitAndroidApplicationContext(const JavaRef<jobject>& context);

const jobject GetAndroidApplicationContext();

}  // namespace jni
}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_ANDROID_JNI_UTIL_H_
