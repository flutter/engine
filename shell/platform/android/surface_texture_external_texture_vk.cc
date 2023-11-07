// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/surface_texture_external_texture_vk.h"

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
  if (state_ == AttachmentState::kUninitialized) {
    texture_ = std::make_shared<impeller::TextureVK>(impeller_context_);
  }
}

void SurfaceTextureExternalTextureImpellerVK::Detach() {}

}  // namespace flutter
