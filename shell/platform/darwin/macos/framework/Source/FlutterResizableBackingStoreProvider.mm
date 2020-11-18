// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"

#import <Metal/Metal.h>
#import <OpenGL/gl.h>
#import <QuartzCore/QuartzCore.h>

#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

@implementation FlutterOpenGLResizableBackingStoreProvider {
  NSOpenGLContext* _mainContext;
  id<FlutterSurfaceManager> _surfaceManager;
}

- (instancetype)initWithMainContext:(NSOpenGLContext*)mainContext andLayer:(CALayer*)layer {
  self = [super init];
  if (self) {
    _mainContext = mainContext;
    _surfaceManager = [[FlutterGLSurfaceManager alloc] initWithLayer:layer
                                                       openGLContext:_mainContext];
  }
  return self;
}

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size {
  [_surfaceManager ensureSurfaceSize:size];
}

- (FlutterBackingStoreDescriptor*)getBackingStore {
  return [_surfaceManager getBackingStore];
}

- (void)resizeSynchronizerFlush:(nonnull FlutterResizeSynchronizer*)synchronizer {
  MacOSGLContextSwitch context_switch(_mainContext);
  glFlush();
}

- (void)resizeSynchronizerCommit:(nonnull FlutterResizeSynchronizer*)synchronizer {
  [CATransaction begin];
  [CATransaction setDisableActions:YES];

  [_surfaceManager swapBuffers];

  [CATransaction commit];
}

@end

@implementation FlutterMetalResizableBackingStoreProvider {
  id<MTLDevice> _mtlDevice;
  id<MTLCommandQueue> _mtlCommandQueue;
  id<FlutterSurfaceManager> _surfaceManager;
}

- (instancetype)initWithDevice:(id<MTLDevice>)mtlDevice
                      andQueue:(id<MTLCommandQueue>)mtlCommandQueue
               andCAMetalLayer:(CAMetalLayer*)layer {
  self = [super init];
  if (self) {
    _mtlDevice = mtlDevice;
    _mtlCommandQueue = mtlCommandQueue;
    _surfaceManager = [[FlutterMetalSurfaceManager alloc] initWithDevice:mtlDevice
                                                         andCommandQueue:mtlCommandQueue
                                                         andCAMetalLayer:layer];
  }
  return self;
}

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size {
  // [_surfaceManager ensureSurfaceSize:size];
}

- (FlutterBackingStoreDescriptor*)getBackingStore {
  // int fboId = [_surfaceManager glFrameBufferId];
  // return [[FlutterBackingStoreDescriptor alloc] initOpenGLDescriptorWithFBOId:fboId];
  return nil;
}

- (void)resizeSynchronizerFlush:(nonnull FlutterResizeSynchronizer*)synchronizer {
  // TODO XXX
}

- (void)resizeSynchronizerCommit:(nonnull FlutterResizeSynchronizer*)synchronizer {
  // [CATransaction begin];
  // [CATransaction setDisableActions:YES];

  // [_surfaceManager swapBuffers];

  // [CATransaction commit];
}

@end
