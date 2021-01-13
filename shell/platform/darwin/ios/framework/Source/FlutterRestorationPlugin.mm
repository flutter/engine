// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterRestorationPlugin.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "flutter/fml/logging.h"

FLUTTER_ASSERT_NOT_ARC

@implementation FlutterRestorationPlugin {
  BOOL _waitForData;
  BOOL _restorationEnabled;
  FlutterResult _pendingRequest;
}

- (instancetype)init {
  @throw([NSException
      exceptionWithName:@"FlutterRestorationPlugin must initWithChannel:restorationEnabled:"
                 reason:nil
               userInfo:nil]);
}

- (instancetype)initWithChannel:(FlutterMethodChannel*)channel
             restorationEnabled:(BOOL)restorationEnabled {
  FML_DCHECK(channel) << "channel must be set";
  self = [super init];
  if (self) {
    [channel setMethodCallHandler:^(FlutterMethodCall* call, FlutterResult result) {
      [self handleMethodCall:call result:result];
    }];
    _restorationEnabled = restorationEnabled;
    _waitForData = restorationEnabled;
  }
  return self;
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  if ([[call method] isEqualToString:@"put"]) {
    FlutterStandardTypedData* data = [call arguments];
    _restorationData = [data data];
    result(nil);
  } else if ([[call method] isEqualToString:@"get"]) {
    if (!_restorationEnabled || !_waitForData) {
      result([self dataForFramework]);
      return;
    }
    _pendingRequest = [result retain];
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (void)setRestorationData:(NSData*)data {
  _restorationData = data;
  _waitForData = NO;
  if (_pendingRequest != nil) {
    _pendingRequest([self dataForFramework]);
    [_pendingRequest release];
    _pendingRequest = nil;
  }
}

- (void)markRestorationComplete {
  _waitForData = NO;
  if (_pendingRequest != nil) {
    NSAssert(_restorationEnabled, @"No request can be pending when restoration is disabled.");
    _pendingRequest([self dataForFramework]);
    [_pendingRequest release];
    _pendingRequest = nil;
  }
}

- (void)reset {
  _restorationData = nil;
  if (_pendingRequest != nil) {
    [_pendingRequest release];
  }
  _pendingRequest = nil;
}

- (NSDictionary*)dataForFramework {
  if (!_restorationEnabled) {
    return @{@"enabled" : @NO};
  }
  if (_restorationData == nil) {
    return @{@"enabled" : @YES};
  }
  return @{
    @"enabled" : @YES,
    @"data" : [FlutterStandardTypedData typedDataWithBytes:_restorationData]
  };
}

- (void)dealloc {
  if (_restorationData != nil) {
    [_restorationData release];
  }
  if (_pendingRequest != nil) {
    [_pendingRequest release];
  }
  [super dealloc];
}

@end
