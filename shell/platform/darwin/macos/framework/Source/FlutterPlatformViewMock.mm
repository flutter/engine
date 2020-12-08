// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViewMock.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViews.h"

@implementation MockPlatformView

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  return self;
}

@end

@implementation FlutterPlatformViewMock

- (instancetype)initWithFrame:(CGRect)frame arguments:(id _Nullable)args {
  if (self = [super init]) {
    _view = [[MockPlatformView alloc] initWithFrame:frame];
  }
  return self;
}

@end

@implementation FlutterPlatformViewMockFactory
- (NSObject<FlutterPlatformView>*)createWithFrame:(CGRect)frame
                                   viewIdentifier:(int64_t)viewId
                                        arguments:(id _Nullable)args {
  return [[FlutterPlatformViewMock alloc] initWithFrame:frame arguments:args];
}

- (NSObject<FlutterMessageCodec>*)createArgsCodec {
  return [FlutterStandardMessageCodec sharedInstance];
}

@end
