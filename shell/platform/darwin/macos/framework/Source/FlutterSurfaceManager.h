// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"

@protocol FlutterSurfaceManager

- (void)ensureSurfaceSize:(CGSize)size;

- (void)swapBuffers;

- (nonnull FlutterBackingStoreDescriptor*)renderBufferDescriptor;

@end

@interface FlutterGLSurfaceManager : NSObject <FlutterSurfaceManager>

- (nullable instancetype)initWithLayer:(nonnull CALayer*)containingLayer
                         openGLContext:(nonnull NSOpenGLContext*)opengLContext;

@end

@interface FlutterMetalSurfaceManager : NSObject <FlutterSurfaceManager>

- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)device
                           commandQueue:(nonnull id<MTLCommandQueue>)commandQueue
                             metalLayer:(nonnull CAMetalLayer*)layer;

@end
