// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_external_texture_gl_share_context.h"

#include <GLES/glext.h>

#include "flutter/shell/platform/android/platform_view_android_jni.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"

#include "flutter/fml/trace_event.h"

namespace flutter {
// This is another solution for Flutter's ExternalTexture.
// The original ExternalTexture uses SurfaceTexture to update the frame data
// that native video object produces to an OpenGL texture. In this scheme, we
// directly pass an OpenGL texture ID to the ExternalTexture object, and avoid
// the performance consumption of data writing to SurfaceTexture
AndroidExternalTextureShareContext::AndroidExternalTextureShareContext(
    int64_t id,
    int64_t shareTextureID)
    : Texture(id), texture_id_(shareTextureID) {}

AndroidExternalTextureShareContext::~AndroidExternalTextureShareContext() {}

void AndroidExternalTextureShareContext::OnGrContextCreated() {}

void AndroidExternalTextureShareContext::MarkNewFrameAvailable() {}

void AndroidExternalTextureShareContext::Paint(SkCanvas& canvas,
                                               const SkRect& bounds,
                                               bool freeze,
                                               GrContext* context) {
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, texture_id_,
                                 GL_RGBA8_OES};

  textureInfo.fTarget = GL_TEXTURE_2D;
  transform.setIdentity();

  GrBackendTexture backendTexture(bounds.width(), bounds.height(),
                                  GrMipMapped::kNo, textureInfo);
  sk_sp<SkImage> image = SkImage::MakeFromTexture(
      canvas.getGrContext(), backendTexture, kTopLeft_GrSurfaceOrigin,
      kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
  if (image) {
    canvas.drawImage(image, bounds.x(), bounds.y());
  } else {
    FML_LOG(ERROR) << "Create SKIImage Fail !!";
  }
}

void AndroidExternalTextureShareContext::OnGrContextDestroyed() {}
}  // namespace flutter
