// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_context_gl.h"

#import <OpenGLES/EAGL.h>

#include "flutter/fml/thread_local.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/shell/platform/darwin/ios/ios_external_texture_gl.h"

namespace flutter {

IOSContextGL::IOSContextGL() {
  EAGLContext* resource_eagl_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
  if (resource_eagl_context == nil) {
    resource_eagl_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  }
  resource_eagl_context.sharegroup.debugLabel = @"flutter";
  resource_context_.reset(resource_eagl_context);
  EAGLContext* eagl_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                                    sharegroup:resource_eagl_context.sharegroup];
  if (eagl_context == nil) {
    eagl_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
                                         sharegroup:resource_eagl_context.sharegroup];
  }
  context_.reset(eagl_context);
}

IOSContextGL::~IOSContextGL() = default;

std::unique_ptr<IOSRenderTargetGL> IOSContextGL::CreateRenderTarget(
    fml::scoped_nsobject<CAEAGLLayer> layer) {
  return std::make_unique<IOSRenderTargetGL>(std::move(layer), context_);
}

// |IOSContext|
sk_sp<GrContext> IOSContextGL::CreateResourceContext() {
  if (![EAGLContext setCurrentContext:resource_context_.get()]) {
    FML_DLOG(INFO) << "Could not make resource context current on IO thread. Async texture uploads "
                      "will be disabled. On Simulators, this is expected.";
    return nullptr;
  }

  return ShellIOManager::CreateCompatibleResourceLoadingContext(
      GrBackend::kOpenGL_GrBackend, GPUSurfaceGLDelegate::GetDefaultPlatformGLInterface());
}

// |IOSContext|
std::unique_ptr<GLContextResult> IOSContextGL::MakeCurrent() {
  return std::make_unique<GLContextSwitch>(std::make_unique<IOSSwitchableGLContext>(*context_));
}

// |IOSContext|
std::unique_ptr<Texture> IOSContextGL::CreateExternalTexture(
    int64_t texture_id,
    fml::scoped_nsobject<NSObject<FlutterTexture>> texture) {
  return std::make_unique<IOSExternalTextureGL>(texture_id, std::move(texture));
}

}  // namespace flutter
