// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"

#include "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#include "flutter/testing/testing.h"
#import "third_party/ocmock/Source/OCMock/OCMock.h"

namespace flutter::testing {

// Returns a mock FlutterViewController that is able to work in environments
// without a real pasteboard.
id mockViewController() {
  NSString* fixtures = @(testing::GetFixturesPath());
  FlutterDartProject* project = [[FlutterDartProject alloc]
      initWithAssetsPath:fixtures
             ICUDataPath:[fixtures stringByAppendingString:@"/icudtl.dat"]];
  FlutterViewController* viewController = [[FlutterViewController alloc] initWithProject:project];

  // Mock pasteboard so that this test will work in environments without a
  // real pasteboard.
  id pasteboardMock = OCMClassMock([NSPasteboard class]);
  __block NSString* clipboardString = @"";
  OCMStub([pasteboardMock setString:[OCMArg any] forType:[OCMArg any]])
      .andDo(^(NSInvocation* invocation) {
        [invocation getArgument:&clipboardString atIndex:2];
      });
  OCMExpect([pasteboardMock stringForType:[OCMArg any]]).andDo(^(NSInvocation* invocation) {
    [invocation setReturnValue:&clipboardString];
  });
  id viewControllerMock = OCMPartialMock(viewController);
  OCMStub([viewControllerMock pasteboard]).andReturn(pasteboardMock);
  return viewControllerMock;
}

TEST(FlutterViewControllerTest, HasStringsWhenPasteboardEmpty) {
  id viewControllerMock = mockViewController();

  // Call setData to make sure that the pasteboard is empty.
  __block bool calledSetClear = false;
  FlutterResult resultSetClear = ^(id result) {
    calledSetClear = true;
  };
  FlutterMethodCall* methodCallSetClear =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setData"
                                        arguments:@{@"text" : [NSNull null]}];
  [viewControllerMock handleMethodCall:methodCallSetClear result:resultSetClear];
  ASSERT_TRUE(calledSetClear);

  // Call hasStrings and expect it to be false.
  __block bool calledAfterClear = false;
  __block bool valueAfterClear;
  FlutterResult resultAfterClear = ^(id result) {
    calledAfterClear = true;
    NSNumber* valueNumber = [result valueForKey:@"value"];
    valueAfterClear = [valueNumber boolValue];
  };
  FlutterMethodCall* methodCallAfterClear =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.hasStrings" arguments:nil];
  [viewControllerMock handleMethodCall:methodCallAfterClear result:resultAfterClear];
  ASSERT_TRUE(calledAfterClear);
  ASSERT_FALSE(valueAfterClear);
}

TEST(FlutterViewControllerTest, HasStringsWhenPasteboardFull) {
  id viewControllerMock = mockViewController();

  // Call setClipboardData to make sure there's a string on the pasteboard.
  __block bool calledSet = false;
  FlutterResult resultSet = ^(id result) {
    calledSet = true;
  };
  FlutterMethodCall* methodCallSet =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setData"
                                        arguments:@{@"text" : @"some string"}];
  [viewControllerMock handleMethodCall:methodCallSet result:resultSet];
  ASSERT_TRUE(calledSet);

  // Call hasStrings and expect it to be true.
  __block bool called = false;
  __block bool value;
  FlutterResult result = ^(id result) {
    called = true;
    NSNumber* valueNumber = [result valueForKey:@"value"];
    value = [valueNumber boolValue];
  };
  FlutterMethodCall* methodCall =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.hasStrings" arguments:nil];
  [viewControllerMock handleMethodCall:methodCall result:result];
  ASSERT_TRUE(called);
  ASSERT_TRUE(value);
}

}  // flutter::testing
