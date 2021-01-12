// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>

/**
 * The type of the backing store, either Metal or OpenGL.
 */
typedef NS_ENUM(NSInteger, FlutterMacOSBackingStoreType) {
  FlutterMacOSBackingStoreTypeMetal,
  FlutterMacOSBackingStoreTypeOpenGL
};

/**
 * A wrapper around backing store handles. On OpenGL, refers to the frame buffer id, and on Metal,
 * holds the reference to the MTLTexture. Will only contain one of Metal or OpenGL backing store
 * handles.
 */
@interface FlutterBackingStoreDescriptor : NSObject

/**
 * The type of the backing store, either Metal or OpenGL. See: FlutterMacOSBackingStoreType.
 */
@property(readonly, nonatomic) FlutterMacOSBackingStoreType backingStoreType;

/**
 * Initializes a backing store descriptor with the specified frame buffer id.
 */
- (nullable instancetype)initOpenGLDescriptorWithFBOId:(uint32_t)fboID;

/**
 * Frame buffer ID referenced by this backing store instance. Only valid when `backingStoreType` is
 * OpenGL.
 */
- (uint32_t)frameBufferId;

/**
 * Initializes a backing store descriptor with the specified MTLTexture.
 */
- (nullable instancetype)initMetalDescriptorWithTexture:(nonnull id<MTLTexture>)texture;

/**
 * MTLTexture referenced by this backing store instance. Only valid when `backingStoreType` is
 * Metal.
 */
- (nonnull id<MTLTexture>)metalTexture;

@end
