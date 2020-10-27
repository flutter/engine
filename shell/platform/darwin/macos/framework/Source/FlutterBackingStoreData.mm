// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreData.h"

#include <OpenGL/gl.h>

@interface FlutterBackingStoreData () {
  bool _isRootView;
}
@end

@implementation FlutterBackingStoreData

- (nullable instancetype)initWithIsRootView:(bool)isRootView {
  if (self = [super init]) {
    _isRootView = isRootView;
  }
  return self;
}

- (bool)isRootView {
  return _isRootView;
}

@end
