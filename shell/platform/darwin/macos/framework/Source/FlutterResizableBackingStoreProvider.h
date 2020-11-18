// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"

NS_ASSUME_NONNULL_BEGIN

@protocol FlutterResizableBackingStoreProvider <FlutterResizeSynchronizerDelegate>

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size;

- (FlutterBackingStoreDescriptor*)getBackingStore;

@end

@interface FlutterOpenGLResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

- (instancetype)initWithMainContext:(NSOpenGLContext*)mainContext andLayer:(nullable CALayer*)layer;

@end

@interface FlutterMetalResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

- (instancetype)initWithDevice:(id<MTLDevice>)mtlDevice
                      andQueue:(id<MTLCommandQueue>)mtlCommandQueue
               andCAMetalLayer:(CAMetalLayer*)layer;

@end

NS_ASSUME_NONNULL_END
