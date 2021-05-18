// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalTextureProvider.h"

#include "flutter/fml/logging.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreData.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterIOSurfaceHolder.h"

namespace flutter {

FlutterMetalCompositor::FlutterMetalCompositor(FlutterViewController* view_controller,
                                               id<MTLDevice> mtl_device)
    : FlutterCompositor(view_controller), mtl_device_(mtl_device) {}

bool FlutterMetalCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                                FlutterBackingStore* backing_store_out) {
  if (!view_controller_) {
    return false;
  }

  CGSize size = CGSizeMake(config->size.width, config->size.height);

  backing_store_out->metal.struct_size = sizeof(FlutterMetalBackingStore);
  backing_store_out->metal.texture.struct_size = sizeof(FlutterMetalTexture);

  if (!frame_started_) {
    StartFrame();
    // If the backing store is for the first layer, return the MTLTexture for the
    // FlutterView.
    FlutterMetalRenderBackingStore* backingStore =
        reinterpret_cast<FlutterMetalRenderBackingStore*>(
            [view_controller_.flutterView backingStoreForSize:size]);
    backing_store_out->metal.texture.texture =
        (__bridge FlutterMetalTextureHandle)backingStore.texture;
  } else {
    FlutterMetalTextureProvider* textureProvider =
        [[FlutterMetalTextureProvider alloc] initWithMTLDevice:mtl_device_];
    FlutterIOSurfaceHolder* io_surface_holder = [FlutterIOSurfaceHolder alloc];
    [io_surface_holder recreateIOSurfaceWithSize:size];
    backing_store_out->metal.texture.texture = (__bridge FlutterMetalTextureHandle)
        [textureProvider createTextureWithSize:size iosurface:[io_surface_holder ioSurface]];

    size_t layer_id = CreateCALayer();
    FlutterBackingStoreData* data =
        [[FlutterBackingStoreData alloc] initWithLayerId:layer_id
                                              fbProvider:nil
                                         ioSurfaceHolder:io_surface_holder];
    backing_store_out->metal.texture.user_data = (__bridge_retained void*)data;
  }

  backing_store_out->type = kFlutterBackingStoreTypeMetal;
  backing_store_out->metal.texture.destruction_callback = [](void* user_data) {};

  return true;
}
bool FlutterMetalCompositor::CollectBackingStore(const FlutterBackingStore* backing_store) {
  // No need to explicitly deallocate the MTLTexture as we have ARC enabled.
  return true;
}

bool FlutterMetalCompositor::Present(const FlutterLayer** layers, size_t layers_count) {
  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];
    FlutterBackingStore* backing_store = const_cast<FlutterBackingStore*>(layer->backing_store);
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        if (backing_store->metal.texture.user_data) {
          FlutterBackingStoreData* backing_store_data =
              (__bridge FlutterBackingStoreData*)backing_store->metal.texture.user_data;

          FlutterIOSurfaceHolder* io_surface_holder = [backing_store_data ioSurfaceHolder];
          size_t layer_id = [backing_store_data layerId];

          CALayer* content_layer = ca_layer_map_[layer_id];

          FML_CHECK(content_layer) << "Unable to find a content layer with layer id " << layer_id;

          content_layer.frame = content_layer.superlayer.bounds;

          IOSurfaceRef io_surface_contents = [io_surface_holder ioSurface];
          [content_layer setContents:(__bridge id)io_surface_contents];
        }
        break;
      }
      case kFlutterLayerContentTypePlatformView:
        // Add functionality in follow up PR.
        FML_LOG(WARNING) << "Presenting PlatformViews not yet supported";
        break;
    };
  }

  // The frame has been presented, prepare FlutterMetalCompositor to
  // render a new frame.
  frame_started_ = false;
  return present_callback_();
}

}  // namespace flutter
