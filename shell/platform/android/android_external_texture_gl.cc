// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_external_texture_gl.h"

#include <memory>
#include <utility>

#include "flutter/display_list/display_list_builder.h"
#include "flutter/display_list/display_list_color_source.h"
#include "flutter/display_list/display_list_image.h"
#include "flutter/display_list/display_list_paint.h"
#include "flutter/display_list/display_list_tile_mode.h"
#include "flutter/impeller/display_list/display_list_image_impeller.h"
#include "flutter/impeller/renderer/backend/gles/texture_gles.h"
#include "fml/logging.h"
#include "impeller/aiks/aiks_context.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "include/core/SkTileMode.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

AndroidExternalTextureGL::AndroidExternalTextureGL(
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& surface_texture,
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
    bool enable_impeller)
    : Texture(id),
      jni_facade_(std::move(jni_facade)),
      surface_texture_(surface_texture),
      transform(SkMatrix::I()),
      enable_impeller_(enable_impeller) {}

AndroidExternalTextureGL::~AndroidExternalTextureGL() {
  if (state_ == AttachmentState::attached) {
    glDeleteTextures(1, &texture_name_);
  }
}

void AndroidExternalTextureGL::OnGrContextCreated() {
  state_ = AttachmentState::uninitialized;
}

void AndroidExternalTextureGL::MarkNewFrameAvailable() {
  new_frame_ready_ = true;
}

void AndroidExternalTextureGL::Paint(PaintContext& context,
                                     const SkRect& bounds,
                                     bool freeze,
                                     const SkSamplingOptions& sampling) {
  if (state_ == AttachmentState::detached) {
    return;
  }
  if (state_ == AttachmentState::uninitialized) {
    glGenTextures(1, &texture_name_);
    Attach(static_cast<jint>(texture_name_));
    state_ = AttachmentState::attached;
  }
  if (!freeze && new_frame_ready_) {
    Update();
    new_frame_ready_ = false;
  }

  if (enable_impeller_) {
    DoPaint(context.builder, context.aiks_context, context.dl_paint, bounds,
            sampling);
  } else {
    DoPaint(context.canvas, context.gr_context, context.sk_paint, bounds,
            sampling);
  }
}

void AndroidExternalTextureGL::DoPaint(DisplayListBuilder* builder,
                                       impeller::AiksContext* aiks_context,
                                       const DlPaint* dl_paint,
                                       const SkRect& bounds,
                                       const SkSamplingOptions& sampling) {
  auto reactor =
      impeller::ContextGLES::Cast(*aiks_context->GetContext()).GetReactor();
  impeller::TextureDescriptor desc;

  desc.storage_mode = impeller::StorageMode::kHostVisible;
  desc.format = impeller::PixelFormat::kR8G8B8A8UNormInt;
  desc.size = {1, 1};
  desc.mip_count = 1;
  impeller::WrappedTextureInfoGLES texture_info{GL_TEXTURE_EXTERNAL_OES,
                                                texture_name_};
  auto texture =
      std::make_shared<impeller::TextureGLES>(reactor, desc, texture_info);
  texture->SetIntent(impeller::TextureIntent::kUploadFromHost);
  sk_sp<DlImage> image = impeller::DlImageImpeller::Make(texture);

  if (image) {
    builder->save();
    builder->translate(bounds.x(), bounds.y() + bounds.height());
    builder->scale(bounds.width(), -bounds.height());

    if (!transform.isIdentity()) {
      auto image_color_source = std::make_shared<DlImageColorSource>(
          image, DlTileMode::kRepeat, DlTileMode::kRepeat, ToDl(sampling),
          &transform);
      DlPaint paint;
      if (dl_paint) {
        paint = *dl_paint;
      }
      paint.setColorSource(image_color_source);
      builder->drawRect(SkRect::MakeWH(1, 1), paint);
    } else {
      builder->drawImage(image, SkPoint::Make(0, 0), ToDl(sampling), dl_paint);
    }
    builder->restore();
  }
}

void AndroidExternalTextureGL::DoPaint(SkCanvas* canvas,
                                       GrDirectContext* gr_context,
                                       const SkPaint* sk_paint,
                                       const SkRect& bounds,
                                       const SkSamplingOptions& sampling) {
  GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, texture_name_,
                                 GL_RGBA8_OES};
  GrBackendTexture backendTexture(1, 1, GrMipMapped::kNo, textureInfo);
  sk_sp<SkImage> image = SkImage::MakeFromTexture(
      gr_context, backendTexture, kTopLeft_GrSurfaceOrigin,
      kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
  if (image) {
    SkAutoCanvasRestore autoRestore(canvas, true);

    // The incoming texture is vertically flipped, so we flip it
    // back. OpenGL's coordinate system has Positive Y equivalent to up, while
    // Skia's coordinate system has Negative Y equvalent to up.
    canvas->translate(bounds.x(), bounds.y() + bounds.height());
    canvas->scale(bounds.width(), -bounds.height());

    if (!transform.isIdentity()) {
      sk_sp<SkShader> shader = image->makeShader(
          SkTileMode::kRepeat, SkTileMode::kRepeat, sampling, transform);

      SkPaint paintWithShader;
      if (sk_paint) {
        paintWithShader = *sk_paint;
      }
      paintWithShader.setShader(shader);
      canvas->drawRect(SkRect::MakeWH(1, 1), paintWithShader);
    } else {
      canvas->drawImage(image, 0, 0, sampling, sk_paint);
    }
  }
}

void AndroidExternalTextureGL::UpdateTransform() {
  jni_facade_->SurfaceTextureGetTransformMatrix(
      fml::jni::ScopedJavaLocalRef<jobject>(surface_texture_), transform);

  // Android's SurfaceTexture transform matrix works on texture coordinate
  // lookups in the range 0.0-1.0, while Skia's Shader transform matrix works on
  // the image itself, as if it were inscribed inside a clip rect.
  // An Android transform that scales lookup by 0.5 (displaying 50% of the
  // texture) is the same as a Skia transform by 2.0 (scaling 50% of the image
  // outside of the virtual "clip rect"), so we invert the incoming matrix.
  SkMatrix inverted;
  if (!transform.invert(&inverted)) {
    FML_LOG(FATAL) << "Invalid SurfaceTexture transformation matrix";
  }
  transform = inverted;
}

void AndroidExternalTextureGL::OnGrContextDestroyed() {
  if (state_ == AttachmentState::attached) {
    Detach();
    glDeleteTextures(1, &texture_name_);
  }
  state_ = AttachmentState::detached;
}

void AndroidExternalTextureGL::Attach(jint textureName) {
  jni_facade_->SurfaceTextureAttachToGLContext(
      fml::jni::ScopedJavaLocalRef<jobject>(surface_texture_), textureName);
}

void AndroidExternalTextureGL::Update() {
  jni_facade_->SurfaceTextureUpdateTexImage(
      fml::jni::ScopedJavaLocalRef<jobject>(surface_texture_));
  UpdateTransform();
}

void AndroidExternalTextureGL::Detach() {
  jni_facade_->SurfaceTextureDetachFromGLContext(
      fml::jni::ScopedJavaLocalRef<jobject>(surface_texture_));
}

void AndroidExternalTextureGL::OnTextureUnregistered() {}

}  // namespace flutter
