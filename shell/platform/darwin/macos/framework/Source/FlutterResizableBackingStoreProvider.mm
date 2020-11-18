// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"

#import <OpenGL/gl.h>
#import <QuartzCore/QuartzCore.h>

#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

@implementation FlutterBackingStoreDescriptor {
  int _fboId;
}

- (instancetype)initOpenGLDescriptorWithFBOId:(int)fboId {
  self = [super init];
  if (self) {
    _fboId = fboId;
    _backingStoreType = FlutterMacOSBackingStoreTypeOpenGL;
  }
  return self;
}

- (int)getFrameBufferId {
  NSAssert(_backingStoreType == FlutterMacOSBackingStoreTypeOpenGL,
           @"Only OpenGL backing stores can request FBO id.");
  return _fboId;
}

@end

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
}

- (instancetype)initWithDevice:(id<MTLDevice>)mtlDevice
                      andQueue:(id<MTLCommandQueue>)mtlCommandQueue {
  self = [super init];
  if (self) {
    _mtlDevice = mtlDevice;
    _mtlCommandQueue = mtlCommandQueue;
  }
  return self;
}

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size {
  // [_surfaceManager ensureSurfaceSize:size];
}

- (FlutterBackingStoreDescriptor*)getBackingStore {
  // int fboId = [_surfaceManager glFrameBufferId];
  // return [[FlutterBackingStoreDescriptor alloc] initOpenGLDescriptorWithFBOId:fboId];
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
