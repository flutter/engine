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


- (void)testIfA11yTraversalOrderMatchesAccessibilityScenario{

  NSPredicate *predicate = [NSPredicate predicateWithBlock:^BOOL(id  _Nullable evaluatedObject, NSDictionary<NSString *,id> * _Nullable bindings) {
    XCUIElement *element = evaluatedObject;
    return element.label.length > 0;
  }];
  XCUIElement *firstElement = [self.application.otherElements elementMatchingPredicate:predicate];
  if (![firstElement waitForExistenceWithTimeout:30]) {
    XCTFail(@"Failed due to not able to find any element with 30 seconds");
  }

  XCUIElementQuery *query = self.application.otherElements;
  XCUIElementQuery *allElementsWithA11yLabelQuery = [query matchingPredicate: predicate];
  NSArray<XCUIElement *> *elements = allElementsWithA11yLabelQuery.allElementsBoundByAccessibilityElement;

  XCTAssertEqual(elements.count, 3);

  NSString *item2Label = elements[0].label;
  NSString *item2Value = elements[0].value;
  XCTAssertEqualObjects(item2Label, @"item2 label");
  XCTAssertEqualObjects(item2Value, @"item2 value");

  NSString *item3Label = elements[1].label;
  NSString *item3Value = elements[1].value;
  XCTAssertEqualObjects(item3Label, @"item3 label");
  XCTAssertEqualObjects(item3Value, @"item3 value");

  NSString *item4Label = elements[2].label;
  NSString *item4Value = elements[2].value;
  XCTAssertEqualObjects(item4Label, @"item4 label");
  XCTAssertEqualObjects(item4Value, @"item4 value");
}

@end
