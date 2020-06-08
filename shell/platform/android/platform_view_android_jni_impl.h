// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_

#include "flutter/shell/platform/android/platform_view_android_jni.h"

namespace flutter {

class PlatformViewAndroidJniImpl : public PlatformViewAndroidJni {
 public:
  PlatformViewAndroidJniImpl();

  ~PlatformViewAndroidJniImpl();

  void FlutterViewHandlePlatformMessage(
      const std::string& channel,
      fml::RefPtr<flutter::PlatformMessage> message,
      int responseId) override;

  void FlutterViewHandlePlatformMessageResponse(
      int responseId,
      std::unique_ptr<fml::Mapping> data) override;

  void FlutterViewUpdateSemantics(std::vector<uint8_t> buffer,
                                  std::vector<std::string> strings) override;

  void FlutterViewUpdateCustomAccessibilityActions(
      std::vector<uint8_t> actions_buffer,
      std::vector<std::string> strings) override;

  void FlutterViewOnFirstFrame() override;

  void FlutterViewOnPreEngineRestart() override;

  void SurfaceTextureAttachToGLContext(int textureId) override;

  void SurfaceTextureUpdateTexImage() override;

  void SurfaceTextureGetTransformMatrix(SkMatrix transform) override;

  void SurfaceTextureDetachFromGLContext() override;

 private:
  // Reference to FlutterJNI.
  const fml::jni::JavaObjectWeakGlobalRef java_object_;
}

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_
