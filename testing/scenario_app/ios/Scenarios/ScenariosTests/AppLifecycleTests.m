// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <XCTest/XCTest.h>
#import "../Scenarios/ScreenBeforeFlutter.h"

@interface AppLifecycleTests : XCTestCase
@end

@implementation AppLifecycleTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;
}

- (void)testLifecycleChannel {
  XCTestExpectation* engineStartedExpectation = [self expectationWithDescription:@"Engine started"];

  ScreenBeforeFlutter* rootVC = [[ScreenBeforeFlutter alloc] initWithEngineRunCompletion:^void () {
    [engineStartedExpectation fulfill];
  }];

  [self waitForExpectations:[NSArray arrayWithObject:engineStartedExpectation] timeout:10];
  UIApplication.sharedApplication.delegate.window.rootViewController = rootVC;
  FlutterEngine* engine = rootVC.engine;

  NSMutableArray* lifecycleExpectations = [NSMutableArray arrayWithCapacity:10];
  NSMutableArray* lifecycleEvents = [NSMutableArray arrayWithCapacity:10];

  [lifecycleExpectations addObject:[[XCTestExpectation alloc] initWithDescription:@"A loading FlutterViewController goes through AppLifecycleState.inactive"]];
  [lifecycleExpectations addObject:[[XCTestExpectation alloc] initWithDescription:@"A loading FlutterViewController goes through AppLifecycleState.resumed"]];

  FlutterViewController* flutterVC = [rootVC showFlutter];
}
@end
