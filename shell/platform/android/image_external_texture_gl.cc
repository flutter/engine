// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/image_external_texture_gl.h"

#include <android/hardware_buffer_jni.h>
#include <android/sensor.h>

#include "flutter/common/graphics/texture.h"
#include "flutter/display_list/effects/dl_color_source.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/impeller/core/formats.h"
#include "flutter/impeller/display_list/dl_image_impeller.h"
#include "flutter/impeller/renderer/backend/gles/texture_gles.h"
#include "flutter/impeller/toolkit/egl/image.h"
#include "flutter/impeller/toolkit/gles/texture.h"
#include "flutter/shell/platform/android/ndk_helpers.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/ganesh/SkImageGanesh.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLTypes.h"

namespace flutter {

ImageExternalTextureGL::ImageExternalTextureGL(
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& image_texture_entry,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : ImageExternalTexture(id, image_texture_entry, jni_facade) {}

void ImageExternalTextureGL::Attach(PaintContext& context) {
  if (state_ == AttachmentState::kUninitialized) {
    state_ = AttachmentState::kAttached;
  }
}

void ImageExternalTextureGL::ProcessFrame(PaintContext& context,
                                          const SkRect& bounds) {
  JavaLocalRef image = AcquireLatestImage();
  if (image.is_null()) {
    return;
  }

  // NOTE: In the following code it is important that old_android_image is
  // not closed until after the update of egl_image_ otherwise the image might
  // be closed before the old EGLImage referencing it has been deleted. After
  // an image is closed the underlying HardwareBuffer may be recycled and used
  // for a future frame.
  JavaLocalRef old_android_image(android_image_);
  android_image_.Reset(image);
  JavaLocalRef hardware_buffer = HardwareBufferFor(image);
  AHardwareBuffer* latest_hardware_buffer = AHardwareBufferFor(hardware_buffer);
  auto key = flutter::NDKHelpers::AHardwareBuffer_getId(latest_hardware_buffer);
  auto existing_image = FindImage(key);
  if (existing_image != nullptr) {
    dl_image_ = existing_image;

    CloseHardwareBuffer(hardware_buffer);
    // IMPORTANT: We have just received a new frame to display so close the
    // previous Java Image so that it is recycled and used for a future frame.
    CloseImage(old_android_image);
    return;
  }

  auto egl_image = CreateEGLImage(latest_hardware_buffer);
  CloseHardwareBuffer(hardware_buffer);
  // IMPORTANT: We have just received a new frame to display so close the
  // previous Java Image so that it is recycled and used for a future frame.
  CloseImage(old_android_image);

  if (!egl_image.is_valid()) {
    return;
  }

  dl_image_ = CreateDlImage(context, bounds, egl_image);
  AddImage(dl_image_, key);
}

void ImageExternalTextureGL::Detach() {
  // TODO: should we clear the cache in detach?
}

impeller::UniqueEGLImageKHR ImageExternalTextureGL::CreateEGLImage(
    AHardwareBuffer* hardware_buffer) {
  if (hardware_buffer == nullptr) {
    return impeller::UniqueEGLImageKHR();
  }

  EGLDisplay display = eglGetCurrentDisplay();
  FML_CHECK(display != EGL_NO_DISPLAY);

  EGLClientBuffer client_buffer =
      NDKHelpers::eglGetNativeClientBufferANDROID(hardware_buffer);
  FML_DCHECK(client_buffer != nullptr);
  if (client_buffer == nullptr) {
    FML_LOG(ERROR) << "eglGetNativeClientBufferAndroid returned null.";
    return impeller::UniqueEGLImageKHR();
  }

  impeller::EGLImageKHRWithDisplay maybe_image =
      impeller::EGLImageKHRWithDisplay{
          eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                            client_buffer, 0),
          display};

  return impeller::UniqueEGLImageKHR(maybe_image);
}

ImageExternalTextureGLSkia::ImageExternalTextureGLSkia(
    const std::shared_ptr<AndroidContextGLSkia>& context,
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& image_texture_entry,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : ImageExternalTextureGL(id, image_texture_entry, jni_facade) {}

void ImageExternalTextureGLSkia::Attach(PaintContext& context) {
  if (state_ == AttachmentState::kUninitialized) {
    // After this call state_ will be AttachmentState::kAttached and egl_image_
    // will have been created if we still have an Image associated with us.
    ImageExternalTextureGL::Attach(context);
    GLuint texture_name;
    glGenTextures(1, &texture_name);
    texture_.reset(impeller::GLTexture{texture_name});
  }
}

void ImageExternalTextureGLSkia::Detach() {
  ImageExternalTextureGL::Detach();
  texture_.reset();
}

void ImageExternalTextureGLSkia::BindImageToTexture(
    const impeller::UniqueEGLImageKHR& image,
    GLuint tex) {
  if (!image.is_valid() || tex == 0) {
    return;
  }
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES,
                               (GLeglImageOES)image.get().image);
}

sk_sp<flutter::DlImage> ImageExternalTextureGLSkia::CreateDlImage(
    PaintContext& context,
    const SkRect& bounds,
    impeller::UniqueEGLImageKHR& egl_image) {
  BindImageToTexture(egl_image, texture_.get().texture_name);
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES,
                                 texture_.get().texture_name, GL_RGBA8_OES};
  auto backendTexture =
      GrBackendTextures::MakeGL(1, 1, skgpu::Mipmapped::kNo, textureInfo);
  return DlImage::Make(SkImages::BorrowTextureFrom(
      context.gr_context, backendTexture, kTopLeft_GrSurfaceOrigin,
      kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr));
}

ImageExternalTextureGLImpeller::ImageExternalTextureGLImpeller(
    const std::shared_ptr<impeller::ContextGLES>& context,
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& image_textury_entry,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : ImageExternalTextureGL(id, image_textury_entry, jni_facade),
      impeller_context_(context) {}

void ImageExternalTextureGLImpeller::Detach() {}

void ImageExternalTextureGLImpeller::Attach(PaintContext& context) {
  if (state_ == AttachmentState::kUninitialized) {
    ImageExternalTextureGL::Attach(context);
  }
}

sk_sp<flutter::DlImage> ImageExternalTextureGLImpeller::CreateDlImage(
    PaintContext& context,
    const SkRect& bounds,
    impeller::UniqueEGLImageKHR& egl_image) {
  impeller::TextureDescriptor desc;
  desc.type = impeller::TextureType::kTextureExternalOES;
  desc.storage_mode = impeller::StorageMode::kDevicePrivate;
  desc.format = impeller::PixelFormat::kR8G8B8A8UNormInt;
  desc.size = {static_cast<int>(bounds.width()),
               static_cast<int>(bounds.height())};
  desc.mip_count = 1;
  auto texture = std::make_shared<impeller::TextureGLES>(
      impeller_context_->GetReactor(), desc,
      impeller::TextureGLES::IsWrapped::kWrapped);
  texture->SetCoordinateSystem(
      impeller::TextureCoordinateSystem::kUploadFromHost);
  if (!texture->Bind()) {
    return nullptr;
  }
  // Associate the hardware buffer image with the texture.
  glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES,
                               (GLeglImageOES)egl_image.get().image);
  return impeller::DlImageImpeller::Make(texture);
}

}  // namespace flutter
