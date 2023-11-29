// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_NDK_HELPERS_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_NDK_HELPERS_H_

#include "flutter/fml/native_library.h"
#include "flutter/fml/platform/android/jni_util.h"

#include "flutter/impeller/toolkit/egl/egl.h"

#include <android/hardware_buffer.h>
#include <jni.h>
#include <media/NdkImageReader.h>
#include <media/NdkMediaError.h>

namespace flutter {

// TODO: Figure out how or where to store these type signatures.
//
// The actual definition (in NdkImageReader.h) is guarded by an __ANDROID_API__
// check, so we can't reference it directly.
typedef void AImageReader_ImageCallback(void* context, AImageReader* reader);
typedef struct AImageReader_ImageListener {
  void* context;
  AImageReader_ImageCallback* onImageAvailable;
} AImageReader_ImageListener;

// A collection of NDK functions that are available depending on the version of
// the Android SDK we are linked with at runtime.
class NDKHelpers {
 public:
  // API Version 26
  static bool HardwareBufferSupported();
  static AHardwareBuffer* AHardwareBuffer_fromHardwareBuffer(
      JNIEnv* env,
      jobject hardwareBufferObj);
  static void AHardwareBuffer_acquire(AHardwareBuffer* buffer);
  static void AHardwareBuffer_release(AHardwareBuffer* buffer);
  static void AHardwareBuffer_describe(AHardwareBuffer* buffer,
                                       AHardwareBuffer_Desc* desc);
  static media_status_t AImageReader_new(int32_t width,
                                         int32_t height,
                                         int32_t format,
                                         int32_t maxImages,
                                         AImageReader** reader);
  static media_status_t AImageReader_setImageListener(
      AImageReader* reader,
      AImageReader_ImageListener* listener);
  static media_status_t AImageReader_getWindow(AImageReader* reader,
                                               ANativeWindow** window);
  static media_status_t AImageReader_acquireLatestImage(AImageReader* reader,
                                                        AImage** image);
  static jobject ANativeWindow_toSurface(JNIEnv* env, ANativeWindow* window);
  static media_status_t AImage_getHardwareBuffer(AImage* image,
                                                 AHardwareBuffer** buffer);
  static EGLClientBuffer eglGetNativeClientBufferANDROID(
      AHardwareBuffer* buffer);

 private:
  static void Init();
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_NDK_HELPERS_H_
