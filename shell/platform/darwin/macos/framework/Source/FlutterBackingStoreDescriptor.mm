// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"

@implementation FlutterBackingStoreDescriptor {
  int _fboId;
  id<MTLTexture> _mtlTexture;
}

- (instancetype)initOpenGLDescriptorWithFBOId:(int)fboId {
  self = [super init];
  if (self) {
    _fboId = fboId;
    _backingStoreType = FlutterMacOSBackingStoreTypeOpenGL;
  }
  return self;
}

- (int)frameBufferId {
  NSAssert(_backingStoreType == FlutterMacOSBackingStoreTypeOpenGL,
           @"Only OpenGL backing stores can request FBO id.");
  return _fboId;
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
