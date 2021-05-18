// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalTextureProvider.h"

#include <Metal/Metal.h>

@interface FlutterMetalTextureProvider () {
  id<MTLDevice> _device;
}
@end

@implementation FlutterMetalTextureProvider
- (instancetype)initWithMTLDevice:(const id<MTLDevice>)device {
  if (self = [super init]) {
    _device = device;
  }
  return self;
}

- (id<MTLTexture>)createTextureWithSize:(CGSize)size iosurface:(nonnull IOSurfaceRef) ioSurface {
  auto texture_descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                          width:size.width
                                                         height:size.height
                                                      mipmapped:NO];
  return [_device newTextureWithDescriptor:texture_descriptor iosurface:ioSurface plane:0];
}

- (void)dealloc {
}

@end
