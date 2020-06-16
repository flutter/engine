// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_JNI_MOCK_JNI_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_JNI_MOCK_JNI_H_

#include <variant>

#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"

namespace flutter {

//------------------------------------------------------------------------------
/// Mock for |PlatformViewAndroidJNI|. This implementation can be used in unit
/// tests without requiring the Android toolchain.
///
class MockJNI final : public PlatformViewAndroidJNI {
 public:
  struct FlutterViewHandlePlatformMessageCall {
    fml::RefPtr<flutter::PlatformMessage> message;
    int responseId;
  };

  struct FlutterViewCreateOverlaySurfaceCall {
    PlatformViewAndroidJNI::OverlayMetadata* metadata;
  };

  using JNICall = std::variant<FlutterViewHandlePlatformMessageCall,
                               FlutterViewCreateOverlaySurfaceCall>;

  MockJNI();

  ~MockJNI() override;

  void FlutterViewHandlePlatformMessage(
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

  void SurfaceTextureAttachToGLContext(JavaWeakGlobalRef surface_texture,
                                       int textureId) override;

  void SurfaceTextureUpdateTexImage(JavaWeakGlobalRef surface_texture) override;

  void SurfaceTextureGetTransformMatrix(JavaWeakGlobalRef surface_texture,
                                        SkMatrix& transform) override;

  void SurfaceTextureDetachFromGLContext(
      JavaWeakGlobalRef surface_texture) override;

  void FlutterViewOnDisplayPlatformView(int view_id,
                                        int x,
                                        int y,
                                        int width,
                                        int height) override;

  void FlutterViewDisplayOverlaySurface(int surface_id,
                                        int x,
                                        int y,
                                        int width,
                                        int height) override;

  void FlutterViewBeginFrame() override;

  void FlutterViewEndFrame() override;

  std::unique_ptr<PlatformViewAndroidJNI::OverlayMetadata>
  FlutterViewCreateOverlaySurface() override;

  // Gets the JNI calls.
  std::vector<JNICall>& GetCalls();

 private:
  std::vector<JNICall> jni_calls_;
  int overlay_surface_id_ = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_JNI_MOCK_JNI_H_
