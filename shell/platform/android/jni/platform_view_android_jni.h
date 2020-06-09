// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_

#include <any>

#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "flutter/lib/ui/window/platform_message.h"

#include "third_party/skia/include/core/SkMatrix.h"

namespace flutter {

//------------------------------------------------------------------------------
/// Allows to call Java code running in the JVM from any thread. However, most
/// methods can only be called from the platform thread as that is where the
/// Java code runs.
///
/// This interface must not depend on the Android toolchain directly, so it can
/// be used in unit tests compiled with the host toolchain.
///
class PlatformViewAndroidJNI {
 public:
  virtual ~PlatformViewAndroidJNI();

  //----------------------------------------------------------------------------
  /// @brief      Sends a platform message. The message may be empty.
  ///
  virtual void FlutterViewHandlePlatformMessage(
      fml::RefPtr<flutter::PlatformMessage> message,
      int responseId) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Responds to a platform message. The data may be a `nullptr`.
  ///
  virtual void FlutterViewHandlePlatformMessageResponse(
      int responseId,
      std::unique_ptr<fml::Mapping> data) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Sends semantics tree updates.
  ///
  /// @note       Must be called from the platform thread.
  ///
  virtual void FlutterViewUpdateSemantics(std::vector<uint8_t> buffer,
                                          std::vector<std::string> strings) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Sends new custom accessibility events.
  ///
  /// @note       Must be called from the platform thread.
  ///
  virtual void FlutterViewUpdateCustomAccessibilityActions(
      std::vector<uint8_t> actions_buffer,
      std::vector<std::string> strings) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Indicates that FlutterView should start painting pixels.
  ///
  /// @note       Must be called from the platform thread.
  ///
  virtual void FlutterViewOnFirstFrame() = 0;

  //----------------------------------------------------------------------------
  /// @brief      Indicates that a hot restart is about to happen.
  ///
  virtual void FlutterViewOnPreEngineRestart() = 0;

  //----------------------------------------------------------------------------
  /// @brief      Attach the SurfaceTexture to the OpenGL ES context that is
  ///             current on the calling thread.
  ///
  /// @note       `surface_texture` must be `fml::jni::JavaObjectWeakGlobalRef`
  ///
  virtual void SurfaceTextureAttachToGLContext(std::any surface_texture,
                                               int textureId) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Updates the texture image to the most recent frame from the
  ///             image stream.
  ///
  /// @note       `surface_texture` must be `fml::jni::JavaObjectWeakGlobalRef`
  ///
  virtual void SurfaceTextureUpdateTexImage(std::any surface_texture) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Gets the transform matrix from the SurfaceTexture.
  ///             Then, it updates the `transform` matrix, so it fill the canvas
  ///             and preserve the aspect ratio.
  ///
  /// @note       `surface_texture` must be `fml::jni::JavaObjectWeakGlobalRef`
  ///
  virtual void SurfaceTextureGetTransformMatrix(std::any surface_texture,
                                                SkMatrix& transform) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Detaches a SurfaceTexture from the OpenGL ES context.
  ///
  /// @note       `surface_texture` must be `fml::jni::JavaObjectWeakGlobalRef`
  ///
  virtual void SurfaceTextureDetachFromGLContext(std::any surface_texture) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Positions and sizes a platform view if using hybrid
  /// composition.
  ///
  /// @note       Must be called from the platform thread.
  ///
  virtual void FlutterViewOnDisplayPlatformView(int view_id,
                                                int x,
                                                int y,
                                                int width,
                                                int height) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_H_
