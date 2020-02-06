// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

static const NSInteger kSecondsToWaitForPlatformView = 30;

@interface PlatformViewGestureRecognizerTests : XCTestCase

@end

@implementation PlatformViewGestureRecognizerTests

- (void)setUp {
  // Put setup code here. This method is called before the invocation of each test method in the
  // class.

  // In UI tests it is usually best to stop immediately when a failure occurs.
  self.continueAfterFailure = NO;

  // In UI tests it’s important to set the initial state - such as interface orientation - required
  // for your tests before they run. The setUp method is a good place to do this.
}

- (void)tearDown {
  // Put teardown code here. This method is called after the invocation of each test method in the
  // class.
}

- (void)testExample {
  // UI tests must launch the application that they test.
  XCUIApplication* app = [[XCUIApplication alloc] init];
  app.launchArguments = @[ @"--gesture", @"--accept", @"--until-touches-ended" ];
  [app launch];

  NSPredicate* predicateToFindPlatformView =
      [NSPredicate predicateWithBlock:^BOOL(id _Nullable evaluatedObject,
                                            NSDictionary<NSString*, id>* _Nullable bindings) {
        XCUIElement* element = evaluatedObject;
        return [element.identifier isEqualToString:@"platform_view"];
      }];
  XCUIElement* platformView =
      [app.otherElements elementMatchingPredicate:predicateToFindPlatformView];
  if (![platformView waitForExistenceWithTimeout:kSecondsToWaitForPlatformView]) {
    NSLog(@"%@", app.debugDescription);
    XCTFail(@"Failed due to not able to find any platformView with %@ seconds",
            @(kSecondsToWaitForPlatformView));
  }

  XCTAssertNotNil(platformView);
  XCTAssertEqualObjects(platformView.label, @"");

  NSPredicate* predicate = [NSPredicate
      predicateWithFormat:@"label == %@",
                          @"-gestureTouchesBegan-gestureTouchesEnded-platformViewTapped"];
  XCTNSPredicateExpectation* expection =
      [[XCTNSPredicateExpectation alloc] initWithPredicate:predicate object:platformView];

  [platformView tap];
  [self waitForExpectations:@[ expection ] timeout:kSecondsToWaitForPlatformView];
  XCTAssertEqualObjects(platformView.label,
                        @"-gestureTouchesBegan-gestureTouchesEnded-platformViewTapped");
}

@end
