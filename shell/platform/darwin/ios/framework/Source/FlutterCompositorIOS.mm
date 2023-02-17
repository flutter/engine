// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder.h"

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterCompositorIOS.h"

namespace flutter {

FlutterCompositorIOS::FlutterCompositorIOS(
    std::shared_ptr<flutter::FlutterPlatformViewsController> platform_views_controller)
    : platform_views_controller_(platform_views_controller) {}

bool FlutterCompositorIOS::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                              FlutterBackingStore* backing_store_out) {
  FlutterView* flutterView = platform_views_controller_.get()->GetFlutterView();
  if (!flutterView) {
    return false;
  }

  CGSize size = CGSizeMake(config->size.width, config->size.height);
  FlutterSurface* surface = [flutterView.surfaceManager surfaceForSize:size];
  memset(backing_store_out, 0, sizeof(FlutterBackingStore));
  backing_store_out->struct_size = sizeof(FlutterBackingStore);
  backing_store_out->type = kFlutterBackingStoreTypeMetal;
  backing_store_out->metal.struct_size = sizeof(FlutterMetalBackingStore);
  backing_store_out->metal.texture = surface.asFlutterMetalTexture;
  return true;
}

bool FlutterCompositorIOS::Present(uint64_t view_id,
                                   const FlutterLayer** layers,
                                   size_t layers_count) {
  FlutterView* flutterView = platform_views_controller_.get()->GetFlutterView();
  if (!flutterView) {
    return false;
  }

  NSMutableArray* surfaces = [NSMutableArray array];
  for (size_t i = 0; i < layers_count; i++) {
    const FlutterLayer* layer = layers[i];
    if (layer->type == kFlutterLayerContentTypeBackingStore) {
      FlutterSurface* surface =
          [FlutterSurface fromFlutterMetalTexture:&layer->backing_store->metal.texture];

      if (surface) {
        FlutterSurfacePresentInfo* info = [[[FlutterSurfacePresentInfo alloc] init] autorelease];
        info.surface = surface;
        info.offset = CGPointMake(layer->offset.x, layer->offset.y);
        info.zIndex = i;
        [surfaces addObject:info];
      }
    }
  }

  [flutterView.surfaceManager present:surfaces
                               notify:^{
                                 PresentPlatformViews(flutterView, layers, layers_count);
                               }];

  return true;
}

void FlutterCompositorIOS::PresentPlatformViews(FlutterView* default_base_view,
                                                const FlutterLayer** layers,
                                                size_t layers_count) {
  FML_DCHECK([[NSThread currentThread] isMainThread]);
  platform_views_controller_->DisposeViews();
  for (size_t i = 0; i < layers_count; i++) {
    FlutterLayer* layer = (FlutterLayer*)layers[i];
    if (layer->type == kFlutterLayerContentTypePlatformView) {
      PresentPlatformView(default_base_view, layer, i);
    }
  }
}

void FlutterCompositorIOS::PresentPlatformView(FlutterView* default_base_view,
                                               const FlutterLayer* layer,
                                               size_t layer_position) {
  platform_views_controller_->CompositeWithEmbedderPlatformViewLayer(layer);
}

}  // namespace flutter
