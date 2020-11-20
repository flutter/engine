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

struct BackingStoreData {
  void* surface_manager;
  bool is_root_view;
};

FlutterMacOSGLCompositor::FlutterMacOSGLCompositor(FlutterViewController* view_controller)
    : view_controller_(view_controller),
      open_gl_context_(view_controller.flutterView.openGLContext) {}

bool FlutterMacOSGLCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                                  FlutterBackingStore* backing_store_out) {
  CGSize size = CGSizeMake(config->size.width, config->size.height);
  BackingStoreData* data = (BackingStoreData*)malloc(sizeof(BackingStoreData));
  if (data) {
    data->is_root_view = config->root_view;
    data->surface_manager = nullptr;
  }

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.target = GL_RGBA8;

  // This is wrong, _flutterView will manage the size for the root layer. For each layer we
  // shouldn't be interfacing with the FlutterView.
  //
  // 1. Root view will ensure the size. Every other view will be a child of the root view
  // layer.
  // 2. Root CA Layer -> {Child Layer 1, Child Layer 2, Child Layer 3, ...}
  //
  // Resizes will be co-ordinated by the root CA Layer.
  if (data->is_root_view) {
    auto fboName = [view_controller_.flutterView frameBufferIDForSize:size];
    NSLog(@"requested a backing store, so we gave %lld", (long long)fboName);
    backing_store_out->open_gl.framebuffer.name = fboName;
  } else {
    FlutterSurfaceManager* surfaceManager =
        [[FlutterSurfaceManager alloc] initWithLayer:view_controller_.flutterView.layer
                                       openGLContext:open_gl_context_
                                     numFramebuffers:1];
    data->surface_manager = (__bridge_retained void*)surfaceManager;

    int kFlutterSurfaceManagerFrontBuffer = 0;
    [surfaceManager recreateIOSurface:kFlutterSurfaceManagerFrontBuffer size:size];
    [surfaceManager backTextureWithIOSurface:kFlutterSurfaceManagerFrontBuffer size:size];
    backing_store_out->open_gl.framebuffer.name = [surfaceManager glFrameBufferFrontId];
  }

  backing_store_out->open_gl.framebuffer.user_data = data;
  backing_store_out->open_gl.framebuffer.destruction_callback = [](void* user_data) {
    if (user_data != nullptr) {
      BackingStoreData* data = (BackingStoreData*)user_data;
      if (data->surface_manager) {
        CFRelease(data->surface_manager);
      }
      free(data);
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
  NSLog(@"Num layers being presented = %u", (unsigned)layers_count);
  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];
    FlutterBackingStore* backing_store = const_cast<FlutterBackingStore*>(layer->backing_store);
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        BackingStoreData* data = (BackingStoreData*)(backing_store->open_gl.framebuffer.user_data);
        FlutterSurfaceManager* surfaceManager =
            (__bridge FlutterSurfaceManager*)(data->surface_manager);

        if (data->is_root_view) {
          NSLog(@"root view!!");
          [view_controller_.flutterView present];
          return true;
        } else {
          NSLog(@"other view!!");
          int kFlutterSurfaceManagerFrontBuffer = 0;
          [surfaceManager setLayerContentWithIOSurface:kFlutterSurfaceManagerFrontBuffer];
          return present_callback_();
        }
      }
      case kFlutterLayerContentTypePlatformView:
        // Add functionality in follow up PR.
        FML_CHECK(false) << "Presenting PlatformViews not yet supported";
        break;
    };
  }
  return false;
}

void FlutterMacOSGLCompositor::SetPresentCallback(
    const FlutterMacOSGLCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}

}  // namespace flutter
