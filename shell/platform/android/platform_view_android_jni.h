// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_

#include "flutter/fml/macros.h"

namespace flutter {

class PlatformViewAndroidJni {
 public:
  virtual void FlutterViewHandlePlatformMessage(
      const std::string& channel,
      fml::RefPtr<flutter::PlatformMessage> message,
      int responseId);

  virtual void FlutterViewHandlePlatformMessageResponse(
      int responseId,
      std::unique_ptr<fml::Mapping> data);

  virtual void FlutterViewUpdateSemantics(std::vector<uint8_t> buffer,
                                          std::vector<std::string> strings);

  virtual void FlutterViewUpdateCustomAccessibilityActions(
      std::vector<uint8_t> actions_buffer,
      std::vector<std::string> strings);

  virtual void FlutterViewOnFirstFrame();

  virtual void FlutterViewOnPreEngineRestart();

  virtual void SurfaceTextureAttachToGLContext(int textureId);

  virtual void SurfaceTextureUpdateTexImage();

  virtual void SurfaceTextureGetTransformMatrix(SkMatrix transform);

  virtual void SurfaceTextureDetachFromGLContext();

  FML_DISALLOW_COPY_AND_ASSIGN(PlatformViewAndroidJni);
}

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_
