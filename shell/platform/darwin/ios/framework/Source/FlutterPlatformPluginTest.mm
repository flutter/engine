// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterPlatformPlugin.h"
#import "flutter/shell/platform/darwin/ios/platform_view_ios.h"

@interface FlutterPlatformPluginTest : XCTestCase
@end

@implementation FlutterPlatformPluginTest

- (void)testClipboardHasCorrectStrings {
  [UIPasteboard generalPasteboard].string = nil;
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  std::unique_ptr<fml::WeakPtrFactory<FlutterEngine>> _weakFactory =
      std::make_unique<fml::WeakPtrFactory<FlutterEngine>>(engine);
  FlutterPlatformPlugin* plugin =
      [[FlutterPlatformPlugin alloc] initWithEngine:_weakFactory->GetWeakPtr()];

  // Set some string to the pasteboard.
  __block bool calledSet = false;
  FlutterResult resultSet = ^(id result) {
    calledSet = true;
  };
  FlutterMethodCall* methodCallSet =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setData"
                                        arguments:@{@"text" : @"some string"}];
  [plugin handleMethodCall:methodCallSet result:resultSet];
  XCTAssertEqual(calledSet, true);

  // Call hasStrings and expect it to be true.
  __block bool called = false;
  __block bool value;
  FlutterResult result = ^(id result) {
    called = true;
    value = [result[@"value"] boolValue];
  };
  FlutterMethodCall* methodCall =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.hasStrings" arguments:nil];
  [plugin handleMethodCall:methodCall result:result];

  XCTAssertEqual(called, true);
  XCTAssertEqual(value, true);

  // Call getData and expect it to be "null"
  __block NSString* text;
  FlutterResult getDataResult = ^(id result) {
    text = result[@"text"];
  };
  FlutterMethodCall* methodCallGetData =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.getData" arguments:@"text/plain"];
  [plugin handleMethodCall:methodCallGetData result:getDataResult];

  XCTAssertEqualObjects(text, @"some string");
}

- (void)testClipboardSetDataToNullDoNotCrash {
  [UIPasteboard generalPasteboard].string = nil;
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  std::unique_ptr<fml::WeakPtrFactory<FlutterEngine>> _weakFactory =
      std::make_unique<fml::WeakPtrFactory<FlutterEngine>>(engine);
  FlutterPlatformPlugin* plugin =
      [[FlutterPlatformPlugin alloc] initWithEngine:_weakFactory->GetWeakPtr()];

  // Set some string to the pasteboard.
  __block bool calledSet = false;
  FlutterResult resultSet = ^(id result) {
    calledSet = true;
  };
  FlutterMethodCall* methodCallSet =
      [FlutterMethodCall methodCallWithMethodName:@"Clipboard.setData"
                                        arguments:@{@"text" : [NSNull null]}];
  [plugin handleMethodCall:methodCallSet result:resultSet];
  XCTAssertEqual(calledSet, true);

  // Call getData and expect it to be "null"
  __block NSString* value;
  FlutterResult result = ^(id result) {
    value = result[@"text"];
  };
  FlutterMethodCall* methodCall = [FlutterMethodCall methodCallWithMethodName:@"Clipboard.getData"
                                                                    arguments:@"text/plain"];
  [plugin handleMethodCall:methodCall result:result];

  XCTAssertEqualObjects(value, @"null");
}

- (void)testPopSystemNavigator {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"test" project:nil];
  [engine runWithEntrypoint:nil];
  FlutterViewController* flutterViewController =
      [[FlutterViewController alloc] initWithEngine:engine nibName:nil bundle:nil];
  UINavigationController* navigationController =
      [[UINavigationController alloc] initWithRootViewController:flutterViewController];
  UITabBarController* tabBarController = [[UITabBarController alloc] init];
  tabBarController.viewControllers = @[ navigationController ];
  std::unique_ptr<fml::WeakPtrFactory<FlutterEngine>> _weakFactory =
      std::make_unique<fml::WeakPtrFactory<FlutterEngine>>(engine);
  FlutterPlatformPlugin* plugin =
      [[FlutterPlatformPlugin alloc] initWithEngine:_weakFactory->GetWeakPtr()];

  id navigationControllerMock = OCMPartialMock(navigationController);
  OCMStub([navigationControllerMock popViewControllerAnimated:YES]);
  // Set some string to the pasteboard.
  __block bool calledSet = false;
  FlutterResult resultSet = ^(id result) {
    calledSet = true;
  };
  FlutterMethodCall* methodCallSet =
      [FlutterMethodCall methodCallWithMethodName:@"SystemNavigator.pop" arguments:@(YES)];
  [plugin handleMethodCall:methodCallSet result:resultSet];
  XCTAssertEqual(calledSet, true);
  OCMVerify([navigationControllerMock popViewControllerAnimated:YES]);
}

@end
