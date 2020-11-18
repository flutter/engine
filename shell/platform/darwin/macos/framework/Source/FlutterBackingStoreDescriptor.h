// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, FlutterMacOSBackingStoreType) {
  FlutterMacOSBackingStoreTypeMetal,
  FlutterMacOSBackingStoreTypeOpenGL
};

@interface FlutterBackingStoreDescriptor : NSObject

@property(readonly, nonatomic) FlutterMacOSBackingStoreType backingStoreType;

- (instancetype)initOpenGLDescriptorWithFBOId:(int)fboId;

- (instancetype)initMetalDescriptorWithTexture:(id<MTLTexture>)texture;

- (int)getFrameBufferId;

@end

NS_ASSUME_NONNULL_END
