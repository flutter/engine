// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"

@protocol FlutterResizableBackingStoreProvider <FlutterResizeSynchronizerDelegate>

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size;

- (nonnull FlutterBackingStoreDescriptor*)backingStoreDescriptor;

@end

@interface FlutterOpenGLResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

- (nullable instancetype)initWithMainContext:(nonnull NSOpenGLContext*)mainContext
                                     caLayer:(nonnull CALayer*)layer;

@end

@interface FlutterMetalResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)mtlDevice
                           commandQueue:(nonnull id<MTLCommandQueue>)mtlCommandQueue
                             metalLayer:(nonnull CAMetalLayer*)layer;

@end
