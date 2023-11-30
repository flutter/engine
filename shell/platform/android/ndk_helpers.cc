// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/ndk_helpers.h"

#include "fml/native_library.h"

#include "flutter/fml/logging.h"

#include <android/hardware_buffer.h>
#include <android/native_window.h>
#include <android/surface_texture.h>
#include <dlfcn.h>
#include <media/NdkMediaError.h>

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
typedef media_status_t (*fp_AImageReader_newWithUsage)(int32_t width,
                                                       int32_t height,
                                                       int32_t format,
                                                       uint64_t usage,
                                                       int32_t maxImages,
                                                       AImageReader** reader);
typedef media_status_t (*fp_AImageReader_setImageListener)(
    AImageReader* reader,
    AImageReader_ImageListener* listener);
typedef media_status_t (*fp_AImageReader_getWindow)(AImageReader* reader,
                                                    ANativeWindow** window);
typedef media_status_t (*fp_AImageReader_acquireNextImage)(AImageReader* reader,
                                                           AImage** image);
typedef jobject (*fp_ANativeWindow_toSurface)(JNIEnv* env,
                                              ANativeWindow* window);
typedef media_status_t (*fp_AImage_getHardwareBuffer)(AImage* image,
                                                      AHardwareBuffer** buffer);
typedef ASurfaceTexture* (*fp_ASurfaceTexture_fromSurfaceTexture)(
    JNIEnv* env,
    jobject surfaceTextureObj);
typedef int (*fp_ASurfaceTexture_attachToGLContext)(ASurfaceTexture* st,
                                                    uint32_t texName);
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
media_status_t (*_AImageReader_newWithUsage)(int32_t width,
                                             int32_t height,
                                             int32_t format,
                                             uint64_t usage,
                                             int32_t maxImages,
                                             AImageReader** reader) = nullptr;
media_status_t (*_AImageReader_setImageListener)(
    AImageReader* reader,
    AImageReader_ImageListener* listener) = nullptr;
media_status_t (*_AImageReader_getWindow)(AImageReader* reader,
                                          ANativeWindow** window) = nullptr;
media_status_t (*_AImageReader_acquireLatestImage)(AImageReader* reader,
                                                   AImage** image) = nullptr;
jobject (*_ANativeWindow_toSurface)(JNIEnv* env,
                                    ANativeWindow* window) = nullptr;
media_status_t (*_AImage_getHardwareBuffer)(AImage* image,
                                            AHardwareBuffer** buffer) = nullptr;
ASurfaceTexture* (*_ASurfaceTexture_fromSurfaceTexture)(
    JNIEnv* env,
    jobject surfaceTextureObj) = nullptr;
int (*_ASurfaceTexture_attachToGLContext)(ASurfaceTexture* st,
                                          uint32_t texName) = nullptr;
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
  _AImageReader_newWithUsage =
      android
          ->ResolveFunction<fp_AImageReader_newWithUsage>(
              "AImageReader_newWithUsage")
          .value_or(nullptr);
  _AImageReader_setImageListener =
      android
          ->ResolveFunction<fp_AImageReader_setImageListener>(
              "AImageReader_setImageListener")
          .value_or(nullptr);
  _AImageReader_getWindow =
      android
          ->ResolveFunction<fp_AImageReader_getWindow>("AImageReader_getWindow")
          .value_or(nullptr);
  _ANativeWindow_toSurface = android
                                 ->ResolveFunction<fp_ANativeWindow_toSurface>(
                                     "ANativeWindow_toSurface")
                                 .value_or(nullptr);
  _AImageReader_acquireLatestImage =
      android
          ->ResolveFunction<fp_AImageReader_acquireNextImage>(
              "AImageReader_acquireLatestImage")
          .value_or(nullptr);
  _AImage_getHardwareBuffer =
      android
          ->ResolveFunction<fp_AImage_getHardwareBuffer>(
              "AImage_getHardwareBuffer")
          .value_or(nullptr);
  _ASurfaceTexture_fromSurfaceTexture =
      android
          ->ResolveFunction<fp_ASurfaceTexture_fromSurfaceTexture>(
              "ASurfaceTexture_fromSurfaceTexture")
          .value_or(nullptr);
  _ASurfaceTexture_attachToGLContext =
      android
          ->ResolveFunction<fp_ASurfaceTexture_attachToGLContext>(
              "ASurfaceTexture_attachToGLContext")
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

media_status_t NDKHelpers::AImageReader_newWithUsage(int32_t width,
                                                     int32_t height,
                                                     int32_t format,
                                                     uint64_t usage,
                                                     int32_t maxImages,
                                                     AImageReader** reader) {
  NDKHelpers::Init();
  FML_CHECK(_AImageReader_newWithUsage != nullptr);
  return _AImageReader_newWithUsage(width, height, format, usage, maxImages,
                                    reader);
}

media_status_t NDKHelpers::AImageReader_setImageListener(
    AImageReader* reader,
    AImageReader_ImageListener* listener) {
  NDKHelpers::Init();
  FML_CHECK(_AImageReader_setImageListener != nullptr);
  return _AImageReader_setImageListener(reader, listener);
}

media_status_t NDKHelpers::AImageReader_getWindow(AImageReader* reader,
                                                  ANativeWindow** window) {
  NDKHelpers::Init();
  FML_CHECK(_AImageReader_getWindow != nullptr);
  return _AImageReader_getWindow(reader, window);
}

media_status_t NDKHelpers::AImageReader_acquireLatestImage(AImageReader* reader,
                                                           AImage** image) {
  NDKHelpers::Init();
  FML_CHECK(_AImageReader_acquireLatestImage != nullptr);
  return _AImageReader_acquireLatestImage(reader, image);
}

jobject NDKHelpers::ANativeWindow_toSurface(JNIEnv* env,
                                            ANativeWindow* window) {
  NDKHelpers::Init();
  FML_CHECK(_ANativeWindow_toSurface != nullptr);
  return _ANativeWindow_toSurface(env, window);
}

media_status_t NDKHelpers::AImage_getHardwareBuffer(AImage* image,
                                                    AHardwareBuffer** buffer) {
  NDKHelpers::Init();
  FML_CHECK(_AImage_getHardwareBuffer != nullptr);
  return _AImage_getHardwareBuffer(image, buffer);
}

ASurfaceTexture* NDKHelpers::ASurfaceTexture_fromSurfaceTexture(
    JNIEnv* env,
    jobject surfaceTextureObj) {
  NDKHelpers::Init();
  FML_CHECK(_ASurfaceTexture_fromSurfaceTexture != nullptr);
  return _ASurfaceTexture_fromSurfaceTexture(env, surfaceTextureObj);
}

int NDKHelpers::ASurfaceTexture_attachToGLContext(ASurfaceTexture* st,
                                                  uint32_t texName) {
  NDKHelpers::Init();
  FML_CHECK(_ASurfaceTexture_attachToGLContext != nullptr);
  return _ASurfaceTexture_attachToGLContext(st, texName);
}

EGLClientBuffer NDKHelpers::eglGetNativeClientBufferANDROID(
    AHardwareBuffer* buffer) {
  NDKHelpers::Init();
  FML_CHECK(_eglGetNativeClientBufferANDROID != nullptr);
  return _eglGetNativeClientBufferANDROID(buffer);
}

}  // namespace flutter
