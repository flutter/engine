// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

@interface FfiPlatformChannelTest : XCTestCase

@end

@implementation FfiPlatformChannelTest

-(void)testFfiPlatformChannelIsCalled {
  XCUIApplication* app = [[XCUIApplication alloc] init];
  app.launchArguments = @[ @"--ffi-platform-channel" ];
  [app launch];
  
  XCUIElement* button = app.buttons[@"ffi-platform-channel"];
  XCTAssertTrue([button waitForExistenceWithTimeout:30.0]);
}

@end
