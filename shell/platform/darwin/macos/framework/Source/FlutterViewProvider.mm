// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

@interface FlutterViewProvider () {
  __weak FlutterEngine* _engine;
}

@end

@implementation FlutterViewProvider

- (instancetype)initWithEngine:(FlutterEngine*)engine {
  self = [super init];
  if (self != nil) {
    _engine = engine;
  }
  return self;
}

- (nullable FlutterView*)getView:(uint64_t)viewId {
  // Only supports the first view, #0. After Flutter supports multi-view, it
  // should get the view according to the ID.
  if (viewId == kFlutterDefaultViewId) {
    return _engine.viewController.flutterView;
  }
  return nil;
}

@end
