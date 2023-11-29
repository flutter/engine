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
#include "impeller/toolkit/egl/config.h"
#include "impeller/toolkit/egl/context.h"
#include "impeller/toolkit/egl/display.h"
#include "impeller/toolkit/egl/surface.h"
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
    // Create an image reader.
    AImageReader* reader = nullptr;
    auto status = NDKHelpers::AImageReader_new(
        /*width=*/bounds.width(),
        /*height=*/bounds.height(),
        /*format=*/AIMAGE_FORMAT_RGBA_8888,
        /*maxImages=*/1,
        /*reader=*/&reader);
    if (status != AMEDIA_OK || !reader) {
      FML_LOG(ERROR) << "Failed to create image reader.";
      return;
    }

    // Add a listener to the image reader.
    auto listener = AImageReader_ImageListener{
        .context = this,
        .onImageAvailable =
            [](void* context, AImageReader* reader) {
              static_cast<SurfaceTextureExternalTextureVK*>(context)
                  ->OnImageAvailable(reader);
            },
    };
    status = NDKHelpers::AImageReader_setImageListener(reader, &listener);
    if (status != AMEDIA_OK) {
      FML_LOG(ERROR) << "Failed to set image listener.";
      return;
    }

    // Get the surface from the image reader.
    ANativeWindow* window = nullptr;
    status = NDKHelpers::AImageReader_getWindow(reader, &window);
    if (status != AMEDIA_OK || !window) {
      FML_LOG(ERROR) << "Failed to get window from image reader.";
      return;
    }
    // JNIEnv* env = fml::jni::AttachCurrentThread();
    // auto surface = NDKHelpers::ANativeWindow_toSurface(env, window);
    // if (!surface) {
    //   FML_LOG(ERROR) << "Failed to get surface from window.";
    //   return;
    // }

    // TODO: Create an EGL display once per context.
    auto egl = impeller::egl::Display();
    if (!egl.IsValid()) {
      FML_LOG(ERROR) << "Failed to create EGL display.";
      return;
    }

    // Create a context.
    auto desc = impeller::egl::ConfigDescriptor{
        .api = impeller::egl::API::kOpenGLES2,
        .samples = impeller::egl::Samples::kOne,
        .color_format = impeller::egl::ColorFormat::kRGBA8888,
        .stencil_bits = impeller::egl::StencilBits::kEight,
        .depth_bits = impeller::egl::DepthBits::kZero,
    };
    auto config = egl.ChooseConfig(desc);
    if (!config) {
      FML_LOG(ERROR) << "Failed to choose EGL config.";
      return;
    }
    auto context = egl.CreateContext(*config, nullptr);
    if (!context) {
      FML_LOG(ERROR) << "Failed to create EGL context.";
      return;
    }

    // Create a surface from the image reader.
    surface_ = egl.CreateWindowSurface(*config, window);
    if (!surface_) {
      FML_LOG(ERROR) << "Failed to create EGL surface.";
      return;
    }

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
