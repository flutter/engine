// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"

@implementation FlutterBackingStoreDescriptor {
  uint32_t _fboID;
  id<MTLTexture> _mtlTexture;
}

- (instancetype)initOpenGLDescriptorWithFBOId:(uint32_t)fboID {
  self = [super init];
  if (self) {
    _fboID = fboID;
    _backingStoreType = FlutterMacOSBackingStoreTypeOpenGL;
  }
  return self;
}

- (uint32_t)frameBufferId {
  NSAssert(_backingStoreType == FlutterMacOSBackingStoreTypeOpenGL,
           @"Only OpenGL backing stores can request FBO id.");
  return _fboID;
}

- (instancetype)initMetalDescriptorWithTexture:(id<MTLTexture>)texture {
  self = [super init];
  if (self) {
    _mtlTexture = texture;
    _backingStoreType = FlutterMacOSBackingStoreTypeMetal;
  }
  return self;
}

- (id<MTLTexture>)metalTexture {
  NSAssert(_backingStoreType == FlutterMacOSBackingStoreTypeMetal,
           @"Only Metal backing stores can request metal textures.");
  return _mtlTexture;
}

@end
