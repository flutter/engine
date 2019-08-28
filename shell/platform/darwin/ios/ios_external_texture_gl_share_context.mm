// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_external_texture_gl_share_context.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include "flutter/shell/platform/darwin/ios/framework/Source/vsync_waiter_ios.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"

namespace flutter {

IOSExternalTextureShareContext::IOSExternalTextureShareContext(
    int64_t textureId,
    NSObject<FlutterShareTexture>* externalTexture)
    : Texture(textureId), external_texture_(externalTexture) {
  FML_DCHECK(external_texture_);
}

IOSExternalTextureShareContext::~IOSExternalTextureShareContext() = default;

void IOSExternalTextureShareContext::Paint(SkCanvas& canvas,
                                           const SkRect& bounds,
                                           bool freeze,
                                           GrContext* context) {
  GLuint texture_id = [external_texture_ copyShareTexture];
  if (texture_id == 0) {
    return;
  }
  GrGLTextureInfo textureInfo;
  textureInfo.fFormat = GL_RGBA8_OES;
  textureInfo.fID = texture_id;
  textureInfo.fTarget = GL_TEXTURE_2D;

  GrBackendTexture backendTexture(bounds.width(), bounds.height(), GrMipMapped::kNo, textureInfo);
  printf("paint test back\n");
  sk_sp<SkImage> image =
      SkImage::MakeFromTexture(context, backendTexture, kTopLeft_GrSurfaceOrigin,
                               kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
  FML_DCHECK(image) << "Failed to create SkImage from Texture.";
  if (image) {
    canvas.drawImage(image, bounds.x(), bounds.y());
  }
}

void IOSExternalTextureShareContext::OnGrContextCreated() {}

void IOSExternalTextureShareContext::OnGrContextDestroyed() {}

void IOSExternalTextureShareContext::MarkNewFrameAvailable() {}

}  // namespace flutter
