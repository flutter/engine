// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

#import <OpenGL/gl.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.h>

@interface FlutterView () {
  __weak id<FlutterViewReshapeListener> _reshapeListener;
  id<FlutterResizableBackingStoreProvider> _resizableBackingStoreProvider;
  FlutterResizeSynchronizer* _resizeSynchronizer;
}

@end

@implementation FlutterView

#ifdef SHELL_ENABLE_METAL
- (instancetype)initWithMTLDevice:(id<MTLDevice>)device
                     commandQueue:(id<MTLCommandQueue>)commandQueue
                  reshapeListener:(id<FlutterViewReshapeListener>)reshapeListener {
  self = [super initWithFrame:NSZeroRect];
  if (self) {
    _reshapeListener = reshapeListener;

    [self setWantsLayer:YES];
    _resizableBackingStoreProvider = [[FlutterMetalResizableBackingStoreProvider alloc]
         initWithDevice:device
               andQueue:commandQueue
        andCAMetalLayer:reinterpret_cast<CAMetalLayer*>(self.layer)];
    _resizeSynchronizer =
        [[FlutterResizeSynchronizer alloc] initWithDelegate:_resizableBackingStoreProvider];
  }
  return self;
}

+ (Class)layerClass {
  return [CAMetalLayer class];
}

- (CALayer*)makeBackingLayer {
  return [CAMetalLayer layer];
}

#endif

- (instancetype)initWithMainContext:(NSOpenGLContext*)mainContext
                    reshapeListener:(id<FlutterViewReshapeListener>)reshapeListener {
  return [self initWithFrame:NSZeroRect mainContext:mainContext reshapeListener:reshapeListener];
}

- (instancetype)initWithFrame:(NSRect)frame
                  mainContext:(NSOpenGLContext*)mainContext
              reshapeListener:(id<FlutterViewReshapeListener>)reshapeListener {
  self = [super initWithFrame:frame];
  if (self) {
    [self setWantsLayer:YES];

    _resizableBackingStoreProvider =
        [[FlutterOpenGLResizableBackingStoreProvider alloc] initWithMainContext:mainContext
                                                                       andLayer:self.layer];
    _resizeSynchronizer =
        [[FlutterResizeSynchronizer alloc] initWithDelegate:_resizableBackingStoreProvider];
    _reshapeListener = reshapeListener;
  }
  return self;
}

- (FlutterBackingStoreDescriptor*)backingStoreForSize:(CGSize)size {
  if ([_resizeSynchronizer shouldEnsureSurfaceForSize:size]) {
    [_resizableBackingStoreProvider updateBackingStoreIfNecessaryForSize:size];
  }
  return [_resizableBackingStoreProvider getBackingStore];
}

- (void)present {
  [_resizeSynchronizer requestCommit];
}

- (void)reshaped {
  CGSize scaledSize = [self convertSizeToBacking:self.bounds.size];
  [_resizeSynchronizer beginResize:scaledSize
                            notify:^{
                              [_reshapeListener viewDidReshape:self];
                            }];
}

#pragma mark - NSView overrides

- (void)setFrameSize:(NSSize)newSize {
  [super setFrameSize:newSize];
  [self reshaped];
}

/**
 * Declares that the view uses a flipped coordinate system, consistent with Flutter conventions.
 */
- (BOOL)isFlipped {
  return YES;
}

- (BOOL)isOpaque {
  return YES;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)viewDidChangeBackingProperties {
  [super viewDidChangeBackingProperties];
  // Force redraw
  [_reshapeListener viewDidReshape:self];
}

@end
