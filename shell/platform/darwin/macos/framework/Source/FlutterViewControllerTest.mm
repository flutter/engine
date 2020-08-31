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
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setClipboardData"
                                        arguments:@{@"text" : @"some string"}];
  [viewController handleMethodCall:methodCallSet result:resultSet];
  ASSERT_TRUE(calledSet);

  // Call hasStrings and expect it to be true.
  __block bool called = false;
  __block bool value;
  FlutterResult result = ^(id result) {
    called = true;
    value = result[@"value"];
  };
  FlutterMethodCall* methodCall =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.hasStrings"
                                        arguments:nil];
  [viewController handleMethodCall:methodCall result:result];
  ASSERT_TRUE(called);
  ASSERT_TRUE(value);
}

}  // flutter::testing
