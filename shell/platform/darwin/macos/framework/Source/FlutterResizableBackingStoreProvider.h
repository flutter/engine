// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, FlutterMacOSBackingStoreType) {
  FlutterMacOSBackingStoreTypeMetal,
  FlutterMacOSBackingStoreTypeOpenGL
};

@interface FlutterBackingStoreDescriptor : NSObject

@property(readonly, nonatomic) FlutterMacOSBackingStoreType backingStoreType;

- (instancetype)initOpenGLDescriptorWithFBOId:(int)fboId;

- (int)getFrameBufferId;

@end

@protocol FlutterResizableBackingStoreProvider <FlutterResizeSynchronizerDelegate>

- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size;

- (FlutterBackingStoreDescriptor*)getBackingStore;

@end

@interface FlutterOpenGLResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

- (instancetype)initWithMainContext:(NSOpenGLContext*)mainContext andLayer:(nullable CALayer*)layer;

- (void)resizeSynchronizerFlush:(nonnull FlutterResizeSynchronizer*)synchronizer;

- (void)resizeSynchronizerCommit:(nonnull FlutterResizeSynchronizer*)synchronizer;

@end

NS_ASSUME_NONNULL_END
