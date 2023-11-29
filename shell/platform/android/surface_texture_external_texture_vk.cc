// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <media/NdkImageReader.h>
#include <memory>

#include "flutter/fml/platform/android/scoped_java_ref.h"
#include "flutter/impeller/renderer/backend/vulkan/context_vk.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"
#include "flutter/shell/platform/android/surface_texture_external_texture_vk.h"
#include "fml/logging.h"
#include "shell/platform/android/ndk_helpers.h"
#include "shell/platform/android/surface_texture_external_texture.h"

namespace flutter {

SurfaceTextureExternalTextureVK::SurfaceTextureExternalTextureVK(
    const std::shared_ptr<impeller::ContextVK>& context,
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& surface_texture,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : SurfaceTextureExternalTexture(id, surface_texture, jni_facade),
      impeller_context_(context) {}

SurfaceTextureExternalTextureVK::~SurfaceTextureExternalTextureVK() {}

void SurfaceTextureExternalTextureVK::ProcessFrame(PaintContext& context,
                                                   const SkRect& bounds) {
  if (state_ == AttachmentState::kUninitialized) {
    FML_LOG(ERROR) << "Initializing SurfaceTextureExternalTextureVK.";
    AImageReader* image_reader;
    media_status_t status = NDKHelpers::AImageReader_new(
        /*width=*/bounds.width(),
        /*height=*/bounds.height(),
        /*format=*/AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        /*maxImages=*/1,
        /*reader=*/&image_reader);
    if (status != AMEDIA_OK) {
      FML_LOG(ERROR) << "Failed to create ImageReader.";
      return;
    }
    FML_DCHECK(image_reader != nullptr);

    // Configure the ImageReader to use the SurfaceTexture as its source.
    auto native_window = NDKHelpers::AImageReader_getWindow(image_reader);
    if (native_window == nullptr) {
      FML_LOG(ERROR) << "Failed to get native window from ImageReader.";
      return;
    }

    // Create a callback to be notified when a new image is available.
    AImageReader_ImageListener image_listener;
    image_listener.context = this;
    image_listener.onImageAvailable = [](void* context, AImageReader* reader) {
      FML_LOG(ERROR) << "OnImageAvailable";
      static_cast<SurfaceTextureExternalTextureVK*>(context)->OnImageAvailable(
          reader);
    };
    status = NDKHelpers::AImageReader_setImageListener(image_reader,
                                                       &image_listener);
    if (status != AMEDIA_OK) {
      FML_LOG(ERROR) << "Failed to set image listener.";
      return;
    }
    FML_LOG(ERROR) << "Image listener set.";

    // TODO: Remove. This is just to prevent assertion errors.
    state_ = AttachmentState::kAttached;
  }

  // TODO: Blit the image from the SurfaceTexture to the ImageReader.

  // TODO: Take the hardware buffer from the ImageReader annd render as DlImage.
}

void SurfaceTextureExternalTextureVK::OnImageAvailable(AImageReader* reader) {
  FML_LOG(ERROR) << "::OnImageAvailable";
}

void SurfaceTextureExternalTextureVK::Detach() {
  SurfaceTextureExternalTexture::Detach();
}

}  // namespace flutter
