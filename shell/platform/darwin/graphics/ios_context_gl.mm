// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/graphics/ios_context_gl.h"

#import <OpenGLES/EAGL.h>

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#import "flutter/shell/platform/darwin/graphics/ios_external_texture_gl.h"

namespace flutter {

static sk_sp<GrDirectContext> CreateCompatibleResourceLoadingContext(
    sk_sp<const GrGLInterface> gl_interface) {
  GrContextOptions options = {};
  if (PersistentCache::cache_sksl()) {
    FML_LOG(INFO) << "Cache SkSL";
    options.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
  }
  PersistentCache::MarkStrategySet();

  options.fPersistentCache = PersistentCache::GetCacheForProcess();

  // There is currently a bug with doing GPU YUV to RGB conversions on the IO
  // thread. The necessary work isn't being flushed or synchronized with the
  // other threads correctly, so the textures end up blank.  For now, suppress
  // that feature, which will cause texture uploads to do CPU YUV conversion.
  // A similar work-around is also used in shell/gpu/gpu_surface_gl.cc.
  options.fDisableGpuYUVConversion = true;

  // To get video playback on the widest range of devices, we limit Skia to
  // ES2 shading language when the ES3 external image extension is missing.
  options.fPreferExternalImagesOverES3 = true;

  if (auto context = GrDirectContext::MakeGL(gl_interface, options)) {
    // Do not cache textures created by the image decoder.  These textures
    // should be deleted when they are no longer referenced by an SkImage.
    context->setResourceCacheLimits(0, 0);
    return context;
  }

  return nullptr;
}

IOSContextGL::IOSContextGL() {
  resource_context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]);
  if (resource_context_ != nullptr) {
    context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                         sharegroup:resource_context_.get().sharegroup]);
  } else {
    resource_context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]);
    context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
                                         sharegroup:resource_context_.get().sharegroup]);
  }
}

IOSContextGL::~IOSContextGL() = default;

std::unique_ptr<IOSRenderTargetGL> IOSContextGL::CreateRenderTarget(
    fml::scoped_nsobject<CAEAGLLayer> layer) {
  return std::make_unique<IOSRenderTargetGL>(std::move(layer), context_);
}

// |IOSContext|
sk_sp<GrDirectContext> IOSContextGL::CreateResourceContext() {
  if (![EAGLContext setCurrentContext:resource_context_.get()]) {
    FML_DLOG(INFO) << "Could not make resource context current on IO thread. Async texture uploads "
                      "will be disabled. On Simulators, this is expected.";
    return nullptr;
  }

  return CreateCompatibleResourceLoadingContext(
      GPUSurfaceGLDelegate::GetDefaultPlatformGLInterface());
}

// |IOSContext|
std::unique_ptr<GLContextResult> IOSContextGL::MakeCurrent() {
  return std::make_unique<GLContextSwitch>(
      std::make_unique<IOSSwitchableGLContext>(context_.get()));
}

// |IOSContext|
std::unique_ptr<Texture> IOSContextGL::CreateExternalTexture(
    int64_t texture_id,
    fml::scoped_nsobject<NSObject<FlutterTexture>> texture) {
  return std::make_unique<IOSExternalTextureGL>(texture_id, std::move(texture));
}

}  // namespace flutter
