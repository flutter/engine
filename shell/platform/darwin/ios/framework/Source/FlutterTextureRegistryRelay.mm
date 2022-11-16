// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextureRegistryRelay.h"

#include "flutter/fml/logging.h"

@implementation FlutterTextureRegistryRelay : NSObject

#pragma mark - FlutterTextureRegistry

- (instancetype)initWithParent:(NSObject<FlutterTextureRegistry>*)parent {
  self = [super init];
  if (self != nil) {
    self.parent = parent;
  }
  return self;
}

- (int64_t)registerTexture:(NSObject<FlutterTexture>*)texture {
  if (self.parent) {
    return [self.parent registerTexture:texture];
  } else {
    FML_LOG(WARNING) << "Using on an empty registry.";
  }
  return 0;
}

- (void)textureFrameAvailable:(int64_t)textureId {
  if (self.parent) {
    return [self.parent textureFrameAvailable:textureId];
  } else {
    FML_LOG(WARNING) << "Using on an empty registry.";
  }
}

- (void)unregisterTexture:(int64_t)textureId {
  if (self.parent) {
    return [self.parent unregisterTexture:textureId];
  } else {
    FML_LOG(WARNING) << "Using on an empty registry.";
  }
}

@end
