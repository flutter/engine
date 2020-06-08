// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_

#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "flutter/lib/ui/window/platform_message.h"

#include "third_party/skia/include/core/SkMatrix.h"

namespace flutter {

class PlatformViewAndroidJni {
 public:
  virtual ~PlatformViewAndroidJni();

  virtual void FlutterViewHandlePlatformMessage(
      fml::RefPtr<flutter::PlatformMessage> message,
      int responseId) = 0;

  virtual void FlutterViewHandlePlatformMessageResponse(
      int responseId,
      std::unique_ptr<fml::Mapping> data) = 0;

  virtual void FlutterViewUpdateSemantics(std::vector<uint8_t> buffer,
                                          std::vector<std::string> strings) = 0;

  virtual void FlutterViewUpdateCustomAccessibilityActions(
      std::vector<uint8_t> actions_buffer,
      std::vector<std::string> strings) = 0;

  virtual void FlutterViewOnFirstFrame() = 0;

  virtual void FlutterViewOnPreEngineRestart() = 0;

  virtual void SurfaceTextureAttachToGLContext(int textureId) = 0;

  virtual void SurfaceTextureUpdateTexImage() = 0;

  virtual void SurfaceTextureGetTransformMatrix(SkMatrix& transform) = 0;

  virtual void SurfaceTextureDetachFromGLContext() = 0;

  virtual void FlutterViewOnDisplayPlatformView(int view_id,
                                                int x,
                                                int y,
                                                int width,
                                                int height) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_
