// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewEngineProvider.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

@interface FlutterViewEngineProvider () {
  __weak FlutterEngine* _engine;
}

@end

@implementation FlutterViewEngineProvider

- (instancetype)initWithEngine:(FlutterEngine*)engine {
  self = [super init];
  if (self != nil) {
    _engine = engine;
  }
  return self;
}

- (nullable FlutterView*)getView:(uint64_t)viewId {
  // TODO(dkwingsmt): This class only supports the first view for now. After
  // FlutterEngine supports multi-view, it should get the view associated to the
  // ID.
  if (viewId == kFlutterDefaultViewId) {
    return _engine.viewController.flutterView;
  }
  return nil;
}

#pragma mark - FlutterPresenter Embedder callback implementations.

- (FlutterMetalTexture)createTextureForView:(uint64_t)viewId size:(CGSize)size {
  FlutterView* view = [self getView:viewId];
  NSAssert(view != nil, @"Can't create texture on a non-existent view 0x%llx.", viewId);
  if (view == nil) {
    // FlutterMetalTexture has texture `null`, therefore is discarded.
    return FlutterMetalTexture{};
  }
  return [view.surfaceManager surfaceForSize:size].asFlutterMetalTexture;
}

- (BOOL)present:(uint64_t)viewId texture:(const FlutterMetalTexture*)texture {
  FlutterView* view = [self getView:viewId];
  if (view == nil) {
    return NO;
  }
  FlutterSurface* surface = [FlutterSurface fromFlutterMetalTexture:texture];
  if (surface == nil) {
    return NO;
  }
  FlutterSurfacePresentInfo* info = [[FlutterSurfacePresentInfo alloc] init];
  info.surface = surface;
  [view.surfaceManager present:@[ info ] notify:nil];
  return YES;
}

@end
