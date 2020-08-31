// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#include "flutter/testing/testing.h"

namespace flutter::testing {

TEST(FlutterViewControllerTest, MacOSTestTest) {
  NSString* fixtures = @(testing::GetFixturesPath());
  FlutterDartProject* project = [[FlutterDartProject alloc]
      initWithAssetsPath:fixtures
             ICUDataPath:[fixtures stringByAppendingString:@"/icudtl.dat"]];
  FlutterViewController* viewController = [[FlutterViewController alloc] initWithProject:project];

  // First call setClipboardData to put a string on the pasteboard.
  __block bool calledSet = false;
  FlutterResult resultSet = ^(id result) {
    calledSet = true;
  };
  FlutterMethodCall* methodCallSet =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setData"
                                        arguments:@{@"text" : @"some string"}];
  [viewController handleMethodCall:methodCallSet result:resultSet];
  ASSERT_TRUE(calledSet);

  // Call hasStrings and expect it to be true.
  __block bool called = false;
  __block bool value;
  FlutterResult result = ^(id result) {
    called = true;
    NSNumber *valueNumber = [result valueForKey:@"value"];
    value = [valueNumber boolValue];
  };
  FlutterMethodCall* methodCall =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.hasStrings"
                                        arguments:nil];
  [viewController handleMethodCall:methodCall result:result];
  ASSERT_TRUE(called);
  ASSERT_TRUE(value);

  // Now call setData again to clear the pasteboard.
  __block bool calledSetClear = false;
  FlutterResult resultSetClear = ^(id result) {
    calledSetClear = true;
  };
  FlutterMethodCall* methodCallSetClear =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setData"
                                        arguments:@{@"text" : [NSNull null]}];
  [viewController handleMethodCall:methodCallSetClear result:resultSetClear];
  ASSERT_TRUE(calledSetClear);

  // Call hasStrings and expect it to be false.
  __block bool calledAfterClear = false;
  __block bool valueAfterClear;
  FlutterResult resultAfterClear = ^(id result) {
    calledAfterClear = true;
    NSNumber *valueNumber = [result valueForKey:@"value"];
    valueAfterClear = [valueNumber boolValue];
  };
  FlutterMethodCall* methodCallAfterClear =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.hasStrings"
                                        arguments:nil];
  [viewController handleMethodCall:methodCallAfterClear result:resultAfterClear];
  ASSERT_TRUE(calledAfterClear);
  ASSERT_FALSE(valueAfterClear);
}

}  // flutter::testing
