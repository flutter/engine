// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterGLCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreData.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterFrameBufferProvider.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterIOSurfaceHolder.h"

#import <OpenGL/gl.h>
#import "flutter/fml/logging.h"
#import "flutter/fml/platform/darwin/cf_utils.h"
#import "third_party/skia/include/core/SkCanvas.h"
#import "third_party/skia/include/core/SkSurface.h"
#import "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#import "third_party/skia/include/utils/mac/SkCGUtils.h"

#include <unistd.h>

namespace flutter {

FlutterGLCompositor::FlutterGLCompositor(FlutterViewController* view_controller)
    : view_controller_(view_controller),
      open_gl_context_(view_controller.flutterView.openGLContext) {}

bool FlutterGLCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                             FlutterBackingStore* backing_store_out) {
  CGSize size = CGSizeMake(config->size.width, config->size.height);
  FlutterBackingStoreData* data =
      [[FlutterBackingStoreData alloc] initWithIsRootView:config->is_root_view];

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->is_cacheable = true;
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.target = GL_RGBA8;

  if (config->is_root_view) {
    // The root view uses FlutterSurfaceManager and is not cacheable since
    // the fbo id changes on every present.
    backing_store_out->is_cacheable = false;
    auto fbo = [view_controller_.flutterView frameBufferIDForSize:size];
    backing_store_out->open_gl.framebuffer.name = fbo;
  } else {
    FML_CHECK(false) << "Compositor only supports creating a backing store for the root view";
  }

  backing_store_out->open_gl.framebuffer.user_data = (__bridge_retained void*)data;
  backing_store_out->open_gl.framebuffer.destruction_callback = [](void* user_data) {
    if (user_data != nullptr) {
      CFRelease(user_data);
    }
  };

  return true;
}

bool FlutterGLCompositor::CollectBackingStore(const FlutterBackingStore* backing_store) {
  return true;
}

bool FlutterGLCompositor::Present(const FlutterLayer** layers, size_t layers_count) {
  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];
    FlutterBackingStore* backing_store = const_cast<FlutterBackingStore*>(layer->backing_store);
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        FlutterBackingStoreData* data =
            (__bridge FlutterBackingStoreData*)(backing_store->open_gl.framebuffer.user_data);
        if (![data isRootView]) {
          FML_CHECK(false) << "Compositor only supports presenting the root view.";
        }
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

void FlutterGLCompositor::SetPresentCallback(
    const FlutterGLCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}

}  // namespace flutter
