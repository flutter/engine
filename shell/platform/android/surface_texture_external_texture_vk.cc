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
    FML_LOG(ERROR) << "Created ImageReader.";
  }

  // TODO: Blit the image from the SurfaceTexture to the ImageReader.
  // TODO: Take the hardware buffer from the ImageReader annd render as DlImage.
}

void SurfaceTextureExternalTextureVK::Detach() {
  SurfaceTextureExternalTexture::Detach();
}

}  // namespace flutter
