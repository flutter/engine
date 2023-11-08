// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/surface_texture_external_texture_vk.h"
#include "impeller/core/formats.h"
#include "impeller/display_list/dl_image_impeller.h"

namespace flutter {

SurfaceTextureExternalTextureImpellerVK::
SurfaceTextureExternalTextureImpellerVK(
    const std::shared_ptr<impeller::ContextVK>& context,
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& surface_texture,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : SurfaceTextureExternalTexture(id, surface_texture, jni_facade),
      impeller_context_(context) {}

SurfaceTextureExternalTextureImpellerVK::~
SurfaceTextureExternalTextureImpellerVK() {}

void SurfaceTextureExternalTextureImpellerVK::ProcessFrame(
    PaintContext& context,
    const SkRect& bounds) {
  FML_LOG(ERROR) << "SurfaceTextureExternalTextureImpellerVK::ProcessFrame";

  if (state_ == AttachmentState::kUninitialized) {
    // Allocate a Vulkan texture.
    // TODO(matanlurey): Deduplicate parts of this with the GL version.
    auto texture = impeller_context_->GetResourceAllocator()->CreateTexture({
        .storage_mode = impeller::StorageMode::kDevicePrivate,
        .type = impeller::TextureType::kTextureExternalOES,
        .format = impeller::PixelFormat::kR8G8B8A8UNormInt,
        .size = {static_cast<uint32_t>(bounds.width()),
                 static_cast<uint32_t>(bounds.height())},
        .mip_count = 1,
    });
    texture_ = std::static_pointer_cast<impeller::TextureVK>(texture);

    texture_->SetCoordinateSystem(
        impeller::TextureCoordinateSystem::kUploadFromHost);

    // We're going to use GL external memory extensions to create a rendering
    // target that is backed by the Vulkan allocated texture, and then do a blit
    // pass (in GL) to copy the `GL_TEXTURE_EXTERNAL_OES` texture to the Vulkan
    // texture.
  }
  FML_CHECK(state_ == AttachmentState::kAttached);

  // Updates the texture contents and transformation matrix.
  Update();

  dl_image_ = impeller::DlImageImpeller::Make(texture_);
}

void SurfaceTextureExternalTextureImpellerVK::Detach() {
  SurfaceTextureExternalTexture::Detach();
  texture_.reset();
}

}  // namespace flutter
