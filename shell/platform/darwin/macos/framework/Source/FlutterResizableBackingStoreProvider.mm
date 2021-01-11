// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"

#import <OpenGL/gl.h>
#import <QuartzCore/QuartzCore.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

@implementation FlutterOpenGLResizableBackingStoreProvider {
  NSOpenGLContext* _mainContext;
  id<FlutterSurfaceManager> _surfaceManager;
}

- (instancetype)initWithMainContext:(NSOpenGLContext*)mainContext caLayer:(CALayer*)layer {
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

- (FlutterBackingStoreDescriptor*)backingStoreDescriptor {
  return [_surfaceManager renderBufferDescriptor];
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
                  commandQueue:(id<MTLCommandQueue>)mtlCommandQueue
                    metalLayer:(CAMetalLayer*)layer {
  self = [super init];
  if (self) {
    _mtlDevice = mtlDevice;
    _mtlCommandQueue = mtlCommandQueue;
    _surfaceManager = [[FlutterMetalSurfaceManager alloc] initWithDevice:mtlDevice
                                                            commandQueue:mtlCommandQueue
                                                              metalLayer:layer];
  }
  return self;
}

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size {
  [_surfaceManager ensureSurfaceSize:size];
}

- (FlutterBackingStoreDescriptor*)backingStoreDescriptor {
  return [_surfaceManager renderBufferDescriptor];
}

- (void)resizeSynchronizerFlush:(nonnull FlutterResizeSynchronizer*)synchronizer {
  // TODO (kaushikiska): Support smooth resizing on Metal.
}

- (void)resizeSynchronizerCommit:(nonnull FlutterResizeSynchronizer*)synchronizer {
  id<MTLCommandBuffer> commandBuffer = [_mtlCommandQueue commandBuffer];
  [commandBuffer commit];
  [commandBuffer waitUntilScheduled];
  [_surfaceManager swapBuffers];
}

@end
