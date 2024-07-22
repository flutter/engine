// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_

#include "flutter/fml/platform/android/jni_weak_ref.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      Concrete implementation of `PlatformViewAndroidJNI` that is
///             compiled with the Android toolchain.
///
class PlatformViewAndroidJNIImpl final : public PlatformViewAndroidJNI {
 public:
  explicit PlatformViewAndroidJNIImpl(
      const fml::jni::JavaObjectWeakGlobalRef& java_object);

  ~PlatformViewAndroidJNIImpl() override;

  // |PlatformViewAndroidJNI|
  void FlutterViewHandlePlatformMessage(
      std::unique_ptr<flutter::PlatformMessage> message,
      int responseId) override;

  // |PlatformViewAndroidJNI|
  void FlutterViewHandlePlatformMessageResponse(
      int responseId,
      std::unique_ptr<fml::Mapping> data) override;

  // |PlatformViewAndroidJNI|
  void FlutterViewUpdateSemantics(
      std::vector<uint8_t> buffer,
      std::vector<std::string> strings,
      std::vector<std::vector<uint8_t>> string_attribute_args) override;

  // |PlatformViewAndroidJNI|
  void FlutterViewUpdateCustomAccessibilityActions(
      std::vector<uint8_t> actions_buffer,
      std::vector<std::string> strings) override;

  // |PlatformViewAndroidJNI|
  void FlutterViewOnFirstFrame() override;

  // |PlatformViewAndroidJNI|
  void FlutterViewOnPreEngineRestart() override;

  // |PlatformViewAndroidJNI|
  void SurfaceTextureAttachToGLContext(JavaLocalRef surface_texture,
                                       int textureId) override;

  // |PlatformViewAndroidJNI|
  bool SurfaceTextureShouldUpdate(JavaLocalRef surface_texture) override;

  // |PlatformViewAndroidJNI|
  void SurfaceTextureUpdateTexImage(JavaLocalRef surface_texture) override;

  // |PlatformViewAndroidJNI|
  void SurfaceTextureGetTransformMatrix(JavaLocalRef surface_texture,
                                        SkMatrix& transform) override;

  // |PlatformViewAndroidJNI|
  void SurfaceTextureDetachFromGLContext(JavaLocalRef surface_texture) override;

  // |PlatformViewAndroidJNI|
  JavaLocalRef ImageProducerTextureEntryAcquireLatestImage(
      JavaLocalRef image_texture_entry) override;

  // |PlatformViewAndroidJNI|
  JavaLocalRef ImageGetHardwareBuffer(JavaLocalRef image) override;

  // |PlatformViewAndroidJNI|
  void ImageClose(JavaLocalRef image) override;

  // |PlatformViewAndroidJNI|
  void HardwareBufferClose(JavaLocalRef hardware_buffer) override;

  // |PlatformViewAndroidJNI|
  void FlutterViewOnDisplayPlatformView(int view_id,
                                        int x,
                                        int y,
                                        int width,
                                        int height,
                                        int viewWidth,
                                        int viewHeight,
                                        MutatorsStack mutators_stack) override;

  // |PlatformViewAndroidJNI|
  void FlutterViewDisplayOverlaySurface(int surface_id,
                                        int x,
                                        int y,
                                        int width,
                                        int height) override;

  // |PlatformViewAndroidJNI|
  ASurfaceTransaction* createSurfaceControlTransaction() override;

  // |PlatformViewAndroidJNI|
  bool getIsSynchronizedWithViewHierarchy() override;

  // |PlatformViewAndroidJNI|
  void FlutterViewBeginFrame() override;

  // |PlatformViewAndroidJNI|
  void FlutterViewEndFrame() override;

  // |PlatformViewAndroidJNI|
  std::unique_ptr<PlatformViewAndroidJNI::OverlayMetadata>
  FlutterViewCreateOverlaySurface() override;

  // |PlatformViewAndroidJNI|
  void FlutterViewDestroyOverlaySurfaces() override;

  // |PlatformViewAndroidJNI|
  std::unique_ptr<std::vector<std::string>>
  FlutterViewComputePlatformResolvedLocale(
      std::vector<std::string> supported_locales_data) override;

  // |PlatformViewAndroidJNI|
  double GetDisplayRefreshRate() override;

  // |PlatformViewAndroidJNI|
  double GetDisplayWidth() override;

  // |PlatformViewAndroidJNI|
  double GetDisplayHeight() override;

  // |PlatformViewAndroidJNI|
  double GetDisplayDensity() override;

  // |PlatformViewAndroidJNI|
  bool RequestDartDeferredLibrary(int loading_unit_id) override;

  // |PlatformViewAndroidJNI|
  double FlutterViewGetScaledFontSize(double unscaled_font_size,
                                      int configuration_id) const override;

 private:
  // Reference to FlutterJNI object.
  const fml::jni::JavaObjectWeakGlobalRef java_object_;

  FML_DISALLOW_COPY_AND_ASSIGN(PlatformViewAndroidJNIImpl);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_
