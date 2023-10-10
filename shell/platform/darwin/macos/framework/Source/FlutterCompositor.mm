// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMutatorView.h"

#include "flutter/fml/logging.h"

namespace flutter {

FlutterCompositor::FlutterCompositor(id<FlutterViewProvider> view_provider,
                                     FlutterPlatformViewController* platform_view_controller)
    : view_provider_(view_provider),
      platform_view_controller_(platform_view_controller),
      mutator_views_([NSMapTable strongToStrongObjectsMapTable]),
      cursor_coordinators_([NSMapTable weakToStrongObjectsMapTable]) {
  FML_CHECK(view_provider != nullptr) << "view_provider cannot be nullptr";
}

bool FlutterCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                           FlutterBackingStore* backing_store_out) {
  // TODO(dkwingsmt): This class only supports single-view for now. As more
  // classes are gradually converted to multi-view, it should get the view ID
  // from somewhere.
  FlutterView* view = [view_provider_ viewForId:kFlutterImplicitViewId];
  if (!view) {
    return false;
  }

  CGSize size = CGSizeMake(config->size.width, config->size.height);
  FlutterSurface* surface = [view.surfaceManager surfaceForSize:size];
  memset(backing_store_out, 0, sizeof(FlutterBackingStore));
  backing_store_out->struct_size = sizeof(FlutterBackingStore);
  backing_store_out->type = kFlutterBackingStoreTypeMetal;
  backing_store_out->metal.struct_size = sizeof(FlutterMetalBackingStore);
  backing_store_out->metal.texture = surface.asFlutterMetalTexture;
  return true;
}

bool FlutterCompositor::Present(FlutterViewId view_id,
                                const FlutterLayer** layers,
                                size_t layers_count) {
  FlutterView* view = [view_provider_ viewForId:view_id];
  if (!view) {
    return false;
  }

  NSMutableArray* surfaces = [NSMutableArray array];
  for (size_t i = 0; i < layers_count; i++) {
    const FlutterLayer* layer = layers[i];
    if (layer->type == kFlutterLayerContentTypeBackingStore) {
      FlutterSurface* surface =
          [FlutterSurface fromFlutterMetalTexture:&layer->backing_store->metal.texture];

      if (surface) {
        FlutterSurfacePresentInfo* info = [[FlutterSurfacePresentInfo alloc] init];
        info.surface = surface;
        info.offset = CGPointMake(layer->offset.x, layer->offset.y);
        info.zIndex = i;
        FlutterBackingStorePresentInfo* present_info = layer->backing_store_present_info;
        if (present_info != nullptr && present_info->paint_region != nullptr) {
          auto paint_region = present_info->paint_region;
          // Safe because the size of FlutterRect is not expected to change.
          info.paintRegion = std::vector<FlutterRect>(
              paint_region->rects, paint_region->rects + paint_region->rects_count);
        }
        [surfaces addObject:info];
      }
    }
  }

  [view.surfaceManager present:surfaces
                        notify:^{
                          PresentPlatformViews(view, layers, layers_count);
                        }];

  return true;
}

void FlutterCompositor::PresentPlatformViews(FlutterView* default_base_view,
                                             const FlutterLayer** layers,
                                             size_t layers_count) {
  FML_DCHECK([[NSThread currentThread] isMainThread])
      << "Must be on the main thread to present platform views";

  // Active mutator views for this frame.
  NSMutableArray<FlutterMutatorView*>* present_mutators = [NSMutableArray array];

  for (size_t i = 0; i < layers_count; i++) {
    FlutterLayer* layer = (FlutterLayer*)layers[i];
    if (layer->type == kFlutterLayerContentTypePlatformView) {
      FlutterMutatorView* mutator_view = PresentPlatformView(default_base_view, layer, i);
      [present_mutators addObject:mutator_view];

      // Gather all overlay regions above this mutator view.
      [mutator_view resetHitTestRegion];
      for (size_t j = i + 1; j < layers_count; j++) {
        FlutterLayer* overlayLayer = (FlutterLayer*)layers[j];
        if (overlayLayer->type == kFlutterLayerContentTypeBackingStore) {
          auto present_info = overlayLayer->backing_store_present_info;
          if (present_info != nullptr && present_info->paint_region != nullptr) {
            for (size_t k = 0; k < present_info->paint_region->rects_count; k++) {
              FlutterRect flutter_rect = present_info->paint_region->rects[k];
              double scale = default_base_view.layer.contentsScale;
              CGRect rect = CGRectMake(flutter_rect.left / scale, flutter_rect.top / scale,
                                       (flutter_rect.right - flutter_rect.left) / scale,
                                       (flutter_rect.bottom - flutter_rect.top) / scale);
              CGRect intersection = CGRectIntersection(rect, mutator_view.frame);
              if (!CGRectIsNull(intersection)) {
                intersection.origin.x -= mutator_view.frame.origin.x;
                intersection.origin.y -= mutator_view.frame.origin.y;
                [mutator_view addHitTestIgnoreRegion:intersection];
              }
            }
          }
        }
      }
    }
  }

  NSMutableArray<FlutterMutatorView*>* obsolete_mutators =
      [NSMutableArray arrayWithArray:[mutator_views_ objectEnumerator].allObjects];
  [obsolete_mutators removeObjectsInArray:present_mutators];

  for (FlutterMutatorView* mutator in obsolete_mutators) {
    [mutator_views_ removeObjectForKey:mutator.platformView];
    [mutator removeFromSuperview];
  }

  [platform_view_controller_ disposePlatformViews];
}

FlutterMutatorView* FlutterCompositor::PresentPlatformView(FlutterView* default_base_view,
                                                           const FlutterLayer* layer,
                                                           size_t layer_position) {
  FML_DCHECK([[NSThread currentThread] isMainThread])
      << "Must be on the main thread to present platform views";

  int64_t platform_view_id = layer->platform_view->identifier;
  NSView* platform_view = [platform_view_controller_ platformViewWithID:platform_view_id];

  FML_DCHECK(platform_view) << "Platform view not found for id: " << platform_view_id;

  FlutterCursorCoordinator* coordinator = [cursor_coordinators_ objectForKey:default_base_view];
  if (!coordinator) {
    coordinator = [[FlutterCursorCoordinator alloc] initWithFlutterView:default_base_view];
    [cursor_coordinators_ setObject:coordinator forKey:default_base_view];
  }

  FlutterMutatorView* container = [mutator_views_ objectForKey:platform_view];

  if (!container) {
    container = [[FlutterMutatorView alloc] initWithPlatformView:platform_view
                                                cursorCoordiator:coordinator];
    [mutator_views_ setObject:container forKey:platform_view];
    [default_base_view addSubview:container];
  }

  container.layer.zPosition = layer_position;
  [container applyFlutterLayer:layer];

  return container;
}

}  // namespace flutter
