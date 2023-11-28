// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/ndk_helpers.h"

#include "fml/native_library.h"

#include "flutter/fml/logging.h"

#include <android/hardware_buffer.h>
#include <dlfcn.h>

namespace flutter {

namespace {

typedef AHardwareBuffer* (*fp_AHardwareBuffer_fromHardwareBuffer)(
    JNIEnv* env,
    jobject hardwareBufferObj);
typedef void (*fp_AHardwareBuffer_acquire)(AHardwareBuffer* buffer);
typedef void (*fp_AHardwareBuffer_release)(AHardwareBuffer* buffer);
typedef void (*fp_AHardwareBuffer_describe)(AHardwareBuffer* buffer,
                                            AHardwareBuffer_Desc* desc);
typedef media_status_t (*fp_AImageReader_new)(int32_t width,
                                              int32_t height,
                                              int32_t format,
                                              int32_t maxImages,
                                              AImageReader** reader);
typedef EGLClientBuffer (*fp_eglGetNativeClientBufferANDROID)(
    AHardwareBuffer* buffer);

AHardwareBuffer* (*_AHardwareBuffer_fromHardwareBuffer)(
    JNIEnv* env,
    jobject hardwareBufferObj) = nullptr;
void (*_AHardwareBuffer_acquire)(AHardwareBuffer* buffer) = nullptr;
void (*_AHardwareBuffer_release)(AHardwareBuffer* buffer) = nullptr;
void (*_AHardwareBuffer_describe)(AHardwareBuffer* buffer,
                                  AHardwareBuffer_Desc* desc) = nullptr;
media_status_t (*_AImageReader_new)(int32_t width,
                                    int32_t height,
                                    int32_t format,
                                    int32_t maxImages,
                                    AImageReader** reader) = nullptr;
EGLClientBuffer (*_eglGetNativeClientBufferANDROID)(AHardwareBuffer* buffer) =
    nullptr;

std::once_flag init_once;

void InitOnceCallback() {
  static fml::RefPtr<fml::NativeLibrary> android =
      fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(android.get() != nullptr);
  static fml::RefPtr<fml::NativeLibrary> egl =
      fml::NativeLibrary::Create("libEGL.so");
  FML_CHECK(egl.get() != nullptr);
  _eglGetNativeClientBufferANDROID =
      egl->ResolveFunction<fp_eglGetNativeClientBufferANDROID>(
             "eglGetNativeClientBufferANDROID")
          .value_or(nullptr);
  _AHardwareBuffer_fromHardwareBuffer =
      android
          ->ResolveFunction<fp_AHardwareBuffer_fromHardwareBuffer>(
              "AHardwareBuffer_fromHardwareBuffer")
          .value_or(nullptr);
  _AHardwareBuffer_acquire = android
                                 ->ResolveFunction<fp_AHardwareBuffer_acquire>(
                                     "AHardwareBuffer_acquire")
                                 .value_or(nullptr);
  _AHardwareBuffer_release = android
                                 ->ResolveFunction<fp_AHardwareBuffer_release>(
                                     "AHardwareBuffer_release")
                                 .value_or(nullptr);
  _AHardwareBuffer_describe =
      android
          ->ResolveFunction<fp_AHardwareBuffer_describe>(
              "AHardwareBuffer_describe")
          .value_or(nullptr);
  _AImageReader_new =
      android->ResolveFunction<fp_AImageReader_new>("AImageReader_new")
          .value_or(nullptr);
}

}  // namespace

void NDKHelpers::Init() {
  std::call_once(init_once, InitOnceCallback);
}

bool NDKHelpers::HardwareBufferSupported() {
  NDKHelpers::Init();
  const bool r = _AHardwareBuffer_fromHardwareBuffer != nullptr;
  return r;
}

AHardwareBuffer* NDKHelpers::AHardwareBuffer_fromHardwareBuffer(
    JNIEnv* env,
    jobject hardwareBufferObj) {
  NDKHelpers::Init();
  FML_CHECK(_AHardwareBuffer_fromHardwareBuffer != nullptr);
  return _AHardwareBuffer_fromHardwareBuffer(env, hardwareBufferObj);
}

void NDKHelpers::AHardwareBuffer_acquire(AHardwareBuffer* buffer) {
  NDKHelpers::Init();
  FML_CHECK(_AHardwareBuffer_acquire != nullptr);
  _AHardwareBuffer_acquire(buffer);
}

void NDKHelpers::AHardwareBuffer_release(AHardwareBuffer* buffer) {
  NDKHelpers::Init();
  FML_CHECK(_AHardwareBuffer_release != nullptr);
  _AHardwareBuffer_release(buffer);
}

void NDKHelpers::AHardwareBuffer_describe(AHardwareBuffer* buffer,
                                          AHardwareBuffer_Desc* desc) {
  NDKHelpers::Init();
  FML_CHECK(_AHardwareBuffer_describe != nullptr);
  _AHardwareBuffer_describe(buffer, desc);
}

media_status_t NDKHelpers::AImageReader_new(int32_t width,
                                            int32_t height,
                                            int32_t format,
                                            int32_t maxImages,
                                            AImageReader** reader) {
  NDKHelpers::Init();
  FML_CHECK(_AImageReader_new != nullptr);
  return _AImageReader_new(width, height, format, maxImages, reader);
}

EGLClientBuffer NDKHelpers::eglGetNativeClientBufferANDROID(
    AHardwareBuffer* buffer) {
  NDKHelpers::Init();
  FML_CHECK(_eglGetNativeClientBufferANDROID != nullptr);
  return _eglGetNativeClientBufferANDROID(buffer);
}

}  // namespace flutter
