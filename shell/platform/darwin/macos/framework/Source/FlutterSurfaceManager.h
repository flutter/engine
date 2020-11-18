// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"

NS_ASSUME_NONNULL_BEGIN
@protocol FlutterSurfaceManager

- (void)ensureSurfaceSize:(CGSize)size;

- (void)swapBuffers;

- (FlutterBackingStoreDescriptor*)getBackingStore;

@end

// Manages the IOSurfaces for FlutterView
@interface FlutterGLSurfaceManager : NSObject <FlutterSurfaceManager>

- (nullable instancetype)initWithLayer:(CALayer*)containingLayer
                         openGLContext:(NSOpenGLContext*)opengLContext;

@end

@interface FlutterMetalSurfaceManager : NSObject <FlutterSurfaceManager>

- (nullable instancetype)initWithDevice:(id<MTLDevice>)device
                        andCommandQueue:(id<MTLCommandQueue>)commandQueue
                        andCAMetalLayer:(CAMetalLayer*)layer;

@end

NS_ASSUME_NONNULL_END
