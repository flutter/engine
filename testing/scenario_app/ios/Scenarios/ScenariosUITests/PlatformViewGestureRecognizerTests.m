// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

static const NSInteger kSecondsToWaitForPlatformView = 30;

@interface PlatformViewGestureRecognizerTests : XCTestCase

@property(nonatomic, strong) XCUIApplication* application;


@end

@implementation PlatformViewGestureRecognizerTests

- (void)setUp {

    self.continueAfterFailure = NO;

    self.application = [[XCUIApplication alloc] init];
    self.application.launchArguments = @[ @"--block-until-touches-ended" ];
    [self.application launch];
}

- (void)testExample {
  
  XCUIElement* element = self.application.textViews.firstMatch;
  BOOL exists = [element waitForExistenceWithTimeout:kSecondsToWaitForPlatformView];
  if (!exists) {
    XCTFail(@"It took longer than %@ second to find the platform view."
            @"There might be issues with the platform view's construction,"
            @"or with how the scenario is built.",
            @(kSecondsToWaitForPlatformView));
  }
  CGRect rect = element.frame;
  XCUICoordinate *coordinate = [self.application coordinateWithNormalizedOffset:CGVectorMake(rect.origin.x+10, rect.origin.y+10)];
  [coordinate tap];

  NSLog(@"accessibility label %@", element.accessibilityLabel);
}

@end
