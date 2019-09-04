// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/flutter.h>
#import <XCTest/XCTest.h>

@interface AccessibilityUITests : XCTestCase
@property(nonatomic, strong) XCUIApplication* application;
@end

@implementation AccessibilityUITests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--accessibility" ];
  [self.application launch];
}

- (void)testAccessiblity{
  sleep(5);
  NSLog(@"%@",self.application.debugDescription);
}

@end
