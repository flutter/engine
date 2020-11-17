// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMacOSGLCompositor.h"

#import <OpenGL/gl.h>
#import "flutter/fml/logging.h"
#import "flutter/fml/platform/darwin/cf_utils.h"
#import "third_party/skia/include/core/SkCanvas.h"
#import "third_party/skia/include/core/SkSurface.h"
#import "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#import "third_party/skia/include/utils/mac/SkCGUtils.h"

#include <unistd.h>

namespace flutter {

FlutterMacOSGLCompositor::FlutterMacOSGLCompositor(FlutterViewController* view_controller)
    : view_controller_(view_controller),
      open_gl_context_(view_controller.flutterView.openGLContext) {}

bool FlutterMacOSGLCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                                  FlutterBackingStore* backing_store_out) {
  FlutterSurfaceManager* surfaceManager =
      [[FlutterSurfaceManager alloc] initWithLayer:view_controller_.flutterView.layer
                                     openGLContext:open_gl_context_
                                   numFramebuffers:1];

  GLuint fbo = [surfaceManager getFramebuffer];
  GLuint texture = [surfaceManager getTexture];

  CGSize size = CGSizeMake(config->size.width, config->size.height);
  size_t kFlutterSurfaceManagerFrontBuffer = 0;
  [surfaceManager backTextureWithIOSurface:kFlutterSurfaceManagerFrontBuffer
                                      size:size
                            backingTexture:texture
                                       fbo:fbo];

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.target = GL_RGBA8;
  backing_store_out->open_gl.framebuffer.name = fbo;
  backing_store_out->open_gl.framebuffer.user_data = (__bridge_retained void*)surfaceManager;
  backing_store_out->open_gl.framebuffer.destruction_callback = [](void* user_data) {
    if (user_data != nullptr) {
      CFRelease(user_data);
    }
  };

  return true;
}

bool FlutterMacOSGLCompositor::CollectBackingStore(const FlutterBackingStore* backing_store) {
  // The memory for FlutterSurfaceManager is handled in the destruction callback.
  // No other memory has to be collected.
  return true;
}

bool FlutterMacOSGLCompositor::Present(const FlutterLayer** layers, size_t layers_count) {
  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];
    FlutterBackingStore* backing_store = const_cast<FlutterBackingStore*>(layer->backing_store);
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        FlutterSurfaceManager* surfaceManager =
            (__bridge FlutterSurfaceManager*)backing_store->open_gl.framebuffer.user_data;

        CGSize size = CGSizeMake(layer->size.width, layer->size.height);
        [view_controller_.flutterView frameBufferIDForSize:size];
        size_t kFlutterSurfaceManagerFrontBuffer = 0;
        [surfaceManager setLayerContentWithIOSurface:kFlutterSurfaceManagerFrontBuffer];
        break;
      }
      case kFlutterLayerContentTypePlatformView:
        // Add functionality in follow up PR.
        FML_CHECK(false) << "Presenting PlatformViews not yet supported";
        break;
    };
  }
  return present_callback_();
}

void FlutterMacOSGLCompositor::SetPresentCallback(
    const FlutterMacOSGLCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}

}  // namespace flutter
