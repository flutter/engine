// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterPlugins.h"

@implementation FlutterMethodPlugin
- (instancetype)initWithController:(FlutterViewController*)controller
                       channelName:(NSString*)channelName {
  NSAssert(channelName, @"Channel name cannot be nil");
  return [self initWithController:controller
                          channel:[FlutterMethodChannel
                                      methodChannelWithName:channelName
                                            binaryMessenger:controller]];
}

- (instancetype)initWithController:(FlutterViewController*)controller
                           channel:(FlutterMethodChannel*)channel {
  NSAssert(controller, @"Controller cannot be nil");
  NSAssert(channel, @"Channel cannot be nil");
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _controller = controller;
  _channel = channel;
  __block __typeof(self) weakSelf = self;
  [_channel setMethodCallHandler:^(FlutterMethodCall* call,
                                   FlutterResultReceiver result) {
    [weakSelf handleMethodInvocation:call.method
                           arguments:call.arguments
                              result:result];
  }];
  return self;
}

- (void)handleMethodInvocation:(NSString*)method
                     arguments:(id _Nullable)arguments
                        result:(FlutterResultReceiver)result {
  result(FlutterMethodNotImplemented);
}

- (void)dealloc {
  [_channel setMethodCallHandler:nil];
  [super dealloc];
}
@end
