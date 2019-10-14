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

  if (!UIAccessibilityIsVoiceOverRunning()) {
    return;
  }

  [super setUp];
  self.continueAfterFailure = NO;

  self.application = [[XCUIApplication alloc] init];
  self.application.launchArguments = @[ @"--accessibility" ];
  [self.application launch];
}

- (void)testIfA11yTraversalOrderMatchesAccessibilityScenario{

  if (!UIAccessibilityIsVoiceOverRunning()) {
    return;
  }

  NSLog(@"%@", self.application.debugDescription);

  // Finding and testing if the root element exists.
  NSPredicate *predicateToFindRoot = [NSPredicate predicateWithBlock:^BOOL(id  _Nullable evaluatedObject, NSDictionary<NSString *,id> * _Nullable bindings) {
    XCUIElement *element = evaluatedObject;
    return [element.identifier isEqualToString:@"c0"];
  }];
  XCUIElement *firstElement = [self.application.otherElements elementMatchingPredicate:predicateToFindRoot];
  if (![firstElement waitForExistenceWithTimeout:30]) {
    NSLog(@"%@", self.application.debugDescription);
    XCTFail(@"Failed due to not able to find any element with 30 seconds");
  }

  XCUIElementQuery *query = self.application.otherElements;
  XCUIElementQuery *rootQuery = [query matchingPredicate: predicateToFindRoot];

  NSArray<XCUIElement *> *rootElements = rootQuery.allElementsBoundByAccessibilityElement;
  XCTAssertEqual(rootElements.count, 1);
  XCUIElement *rootElement = rootElements.firstObject;
  XCTAssertEqualObjects(rootElement.identifier, @"c0");
  XCTAssertFalse(rootElement.label.length > 0);

  // Root should have 3 children, 0, c1 and 2
  NSArray<XCUIElement *> *rootChildrenElements = [rootElement childrenMatchingType:XCUIElementTypeOther].allElementsBoundByAccessibilityElement;
  XCTAssertEqual(rootChildrenElements.count, 3);

  XCUIElement *element0 = rootChildrenElements[0];
  XCTAssertEqualObjects(element0.identifier, @"0");
  XCTAssertEqualObjects(element0.label, @"item0 label");
  XCTAssertEqualObjects(element0.value, @"item0 value");

  XCUIElement *elementC1 = rootChildrenElements[1];
  XCTAssertEqualObjects(elementC1.identifier, @"c1");
  XCTAssertFalse(elementC1.label.length > 0);

  XCUIElement *element2 = rootChildrenElements[2];
  XCTAssertEqualObjects(element2.identifier, @"2");
  XCTAssertEqualObjects(element2.label, @"item2 label");
  XCTAssertEqualObjects(element2.value, @"item2 value");

  // c1 should have 3 children, 1, 3 and 4
  NSArray<XCUIElement *> *c1ChildrenElements = [elementC1 childrenMatchingType:XCUIElementTypeOther].allElementsBoundByAccessibilityElement;
  XCTAssertEqual(rootChildrenElements.count, 3);

  XCUIElement *element1 = c1ChildrenElements[0];
  XCTAssertEqualObjects(element1.label, @"item1 label");
  XCTAssertEqualObjects(element1.value, @"item1 value");

  XCUIElement *element3 = c1ChildrenElements[1];
  XCTAssertEqualObjects(element3.label, @"item3 label");
  XCTAssertEqualObjects(element3.value, @"item3 value");

  XCUIElement *element4 = c1ChildrenElements[2];
  XCTAssertEqualObjects(element4.label, @"item4 label");
  XCTAssertEqualObjects(element4.value, @"item4 value");
}

@end
