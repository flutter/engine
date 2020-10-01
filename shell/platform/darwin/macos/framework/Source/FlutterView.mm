// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"

#import <OpenGL/gl.h>
#import <QuartzCore/QuartzCore.h>

@interface FlutterView () <FlutterResizeSynchronizerDelegate> {
  __weak id<FlutterViewReshapeListener> _reshapeListener;
  FlutterResizeSynchronizer* resizeSynchronizer;
  FlutterSurfaceManager* surfaceManager;
  BOOL active;
  CALayer* contentLayer;
}

@end

@implementation FlutterView

- (instancetype)initWithShareContext:(NSOpenGLContext*)shareContext
                     reshapeListener:(id<FlutterViewReshapeListener>)reshapeListener {
  return [self initWithFrame:NSZeroRect shareContext:shareContext reshapeListener:reshapeListener];
}

- (instancetype)initWithFrame:(NSRect)frame
                 shareContext:(NSOpenGLContext*)shareContext
              reshapeListener:(id<FlutterViewReshapeListener>)reshapeListener {
  self = [super initWithFrame:frame];
  if (self) {
    self.openGLContext = [[NSOpenGLContext alloc] initWithFormat:shareContext.pixelFormat
                                                    shareContext:shareContext];

    [self setWantsLayer:YES];

    // Layer for content (which will be set by surfaceManager). This is separate from
    // self.layer because it needs to be flipped vertically (using layer.sublayerTransform)
    contentLayer = [[CALayer alloc] init];
    [self.layer addSublayer:contentLayer];

    resizeSynchronizer = [[FlutterResizeSynchronizer alloc] initWithDelegate:self];
    surfaceManager = [[FlutterSurfaceManager alloc] initWithLayer:contentLayer
                                                    openGLContext:self.openGLContext];

    _reshapeListener = reshapeListener;
  }
  return self;
}

- (void)resizeSynchronizerFlush:(FlutterResizeSynchronizer*)synchronizer {
  [self.openGLContext makeCurrentContext];
  glFlush();
  [NSOpenGLContext clearCurrentContext];
}

- (void)resizeSynchronizerCommit:(FlutterResizeSynchronizer*)synchronizer {
  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  self.layer.frame = self.bounds;
  self.layer.sublayerTransform = CATransform3DTranslate(CATransform3DMakeScale(1, -1, 1), 0,
                                                        -self.layer.bounds.size.height, 0);
  contentLayer.frame = self.layer.bounds;

  [surfaceManager swapBuffers];

  [CATransaction commit];
}

- (int)getFrameBufferIdForSize:(CGSize)size {
  if ([resizeSynchronizer shouldEnsureSurfaceForSize:size]) {
    [surfaceManager ensureSurfaceSize:size];
  }
  return [surfaceManager glFrameBufferId];
}

- (void)present {
  [resizeSynchronizer requestCommit];
}

- (void)start {
  active = YES;
  [self reshaped];
}

- (void)reshaped {
  if (active) {
    CGSize scaledSize = [self convertSizeToBacking:self.bounds.size];
    [resizeSynchronizer beginResize:scaledSize
                             notify:^{
                               [_reshapeListener viewDidReshape:self];
                             }];
  }
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
