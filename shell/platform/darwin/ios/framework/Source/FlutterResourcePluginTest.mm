// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterResourcePlugin.h"
#import "third_party/ocmock/Source/OCMock/OCMock.h"

FLUTTER_ASSERT_ARC

@interface FlutterResourcePluginTest : XCTestCase
@end

@implementation FlutterResourcePluginTest

- (void)testReturnsValidDataForIcon {
  FlutterResourcePlugin* plugin = [[FlutterResourcePlugin alloc] init];

  __block NSNumber* scale;
  __block FlutterStandardTypedData* data;
  FlutterResult result = ^(id result) {
    scale = result[@"scale"];
    data = result[@"data"];
  };

  // Ask for a system symbol that exists on iOS 13.
  FlutterMethodCall* methodCall =
      [FlutterMethodCall methodCallWithMethodName:@"SystemImage.load"
                                        arguments:@{@"name" : @"doc", @"size" : @(32)}];
  [plugin handleMethodCall:methodCall result:result];

  XCTAssertTrue([@(2) isEqualToNumber:scale]);
  XCTAssertTrue([data isKindOfClass:[FlutterStandardTypedData class]]);
}

- (void)testReturnsNilForNonExistentIcon {
  FlutterResourcePlugin* plugin = [[FlutterResourcePlugin alloc] init];

  __block id nilResult;
  FlutterResult result = ^(id result) {
    nilResult = result;
  };

  // Ask for a bogus symbol that doesn't exist.
  FlutterMethodCall* methodCall = [FlutterMethodCall
      methodCallWithMethodName:@"SystemImage.load"
                     arguments:@{@"name" : @"something that doesn't exist", @"size" : @(32)}];
  [plugin handleMethodCall:methodCall result:result];

  XCTAssertNil(nilResult);
}

- (void)testReturnsValidDataWhenAskingForWeight {
  FlutterResourcePlugin* plugin = [[FlutterResourcePlugin alloc] init];

  __block NSNumber* scale;
  __block FlutterStandardTypedData* data;
  FlutterResult result = ^(id result) {
    scale = result[@"scale"];
    data = result[@"data"];
  };

  // iOS 13 actually doesn't support weights besides regular. But it should still return real
  // data.
  FlutterMethodCall* methodCall = [FlutterMethodCall
      methodCallWithMethodName:@"SystemImage.load"
                     arguments:@{@"name" : @"doc", @"size" : @(32), @"weight" : @(500)}];
  [plugin handleMethodCall:methodCall result:result];

  XCTAssertTrue([@(2) isEqualToNumber:scale]);
  XCTAssertTrue([data isKindOfClass:[FlutterStandardTypedData class]]);
}

@end
