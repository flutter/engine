// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/jni/mock_jni.h"

namespace flutter {

MockJNI::MockJNI() = default;

MockJNI::~MockJNI() = default;

void MockJNI::FlutterViewHandlePlatformMessage(
    fml::RefPtr<flutter::PlatformMessage> message,
    int responseId) {
  jni_calls_.push_back(
      FlutterViewHandlePlatformMessageCall{message, responseId});
}

void MockJNI::FlutterViewHandlePlatformMessageResponse(
    int responseId,
    std::unique_ptr<fml::Mapping> data) {}

void MockJNI::FlutterViewUpdateSemantics(std::vector<uint8_t> buffer,
                                         std::vector<std::string> strings) {}

void MockJNI::FlutterViewUpdateCustomAccessibilityActions(
    std::vector<uint8_t> actions_buffer,
    std::vector<std::string> strings) {}

void MockJNI::FlutterViewOnFirstFrame() {}

void MockJNI::FlutterViewOnPreEngineRestart() {}

void MockJNI::SurfaceTextureAttachToGLContext(JavaWeakGlobalRef surface_texture,
                                              int textureId) {}

void MockJNI::SurfaceTextureUpdateTexImage(JavaWeakGlobalRef surface_texture) {}

void MockJNI::SurfaceTextureGetTransformMatrix(
    JavaWeakGlobalRef surface_texture,
    SkMatrix& transform) {}

void MockJNI::SurfaceTextureDetachFromGLContext(
    JavaWeakGlobalRef surface_texture) {}

void MockJNI::FlutterViewOnDisplayPlatformView(int view_id,
                                               int x,
                                               int y,
                                               int width,
                                               int height) {}

void MockJNI::FlutterViewDisplayOverlaySurface(int surface_id,
                                               int x,
                                               int y,
                                               int width,
                                               int height) {}

std::vector<MockJNI::JNICall>& MockJNI::GetCalls() {
  return jni_calls_;
}

}  // namespace flutter
