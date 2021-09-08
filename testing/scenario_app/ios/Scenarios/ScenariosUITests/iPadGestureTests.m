// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <XCTest/XCTest.h>

static const NSInteger kSecondsToWaitForFlutterView = 30;

@interface iPadGestureTests : XCTestCase

@end

@implementation iPadGestureTests

- (void)setUp {
  [super setUp];
  self.continueAfterFailure = NO;
}

static BOOL performBoolSelector(id target, SEL selector) {
  NSInvocation* invocation = [NSInvocation
      invocationWithMethodSignature:[[target class] instanceMethodSignatureForSelector:selector]];
  [invocation setSelector:selector];
  [invocation setTarget:target];
  [invocation invoke];
  BOOL returnValue;
  [invocation getReturnValue:&returnValue];
  return returnValue;
}

// TODO(85810): Remove reflection in this test when Xcode version is upgraded to 13.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
- (void)testPointerButtons {
  BOOL supportsPointerInteraction = NO;
  SEL supportsPointerInteractionSelector = @selector(supportsPointerInteraction);
  if ([XCUIDevice.sharedDevice respondsToSelector:supportsPointerInteractionSelector]) {
    supportsPointerInteraction =
        performBoolSelector(XCUIDevice.sharedDevice, supportsPointerInteractionSelector);
  }
  XCTSkipUnless(supportsPointerInteraction, "Device does not support pointer interaction.");
  XCUIApplication* app = [[XCUIApplication alloc] init];
  app.launchArguments = @[ @"--pointer-events" ];
  [app launch];

  NSPredicate* predicateToFindFlutterView =
      [NSPredicate predicateWithBlock:^BOOL(id _Nullable evaluatedObject,
                                            NSDictionary<NSString*, id>* _Nullable bindings) {
        XCUIElement* element = evaluatedObject;
        return [element.identifier hasPrefix:@"flutter_view"];
      }];
  XCUIElement* flutterView = [[app descendantsMatchingType:XCUIElementTypeAny]
      elementMatchingPredicate:predicateToFindFlutterView];
  if (![flutterView waitForExistenceWithTimeout:kSecondsToWaitForFlutterView]) {
    NSLog(@"%@", app.debugDescription);
    XCTFail(@"Failed due to not able to find any flutterView with %@ seconds",
            @(kSecondsToWaitForFlutterView));
  }

  XCTAssertNotNil(flutterView);

  [flutterView tap];
  // Initial add event should have buttons = 0
  XCTAssertTrue(
      [app.textFields[@"0,PointerChange.add,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a normal tap");
  // Normal tap should have buttons = 0, the flutter framework will ensure it has buttons = 1
  XCTAssertTrue(
      [app.textFields[@"1,PointerChange.down,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.down event did not occur for a normal tap");
  XCTAssertTrue(
      [app.textFields[@"2,PointerChange.up,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.up event did not occur for a normal tap");
  SEL rightClick = @selector(rightClick);
  XCTAssertTrue([flutterView respondsToSelector:rightClick],
                @"If supportsPointerInteraction is true, this should be true too.");
  [flutterView performSelector:rightClick];
  // Right-clicking will trigger the hover pointer as well
  XCTAssertTrue(
      [app.textFields[@"3,PointerChange.add,device=1,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a right-click");
  XCTAssertTrue(
      [app.textFields[@"4,PointerChange.add,device=2,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a right-click");
  // Right click should have buttons = 2
  XCTAssertTrue(
      [app.textFields[@"5,PointerChange.down,device=2,buttons=2"] waitForExistenceWithTimeout:1],
      @"PointerChange.down event did not occur for a right-click");
  XCTAssertTrue(
      [app.textFields[@"6,PointerChange.up,device=2,buttons=2"] waitForExistenceWithTimeout:1],
      @"PointerChange.up event did not occur for a right-click");
}

- (void)testPointerHover {
  BOOL supportsPointerInteraction = NO;
  SEL supportsPointerInteractionSelector = @selector(supportsPointerInteraction);
  if ([XCUIDevice.sharedDevice respondsToSelector:supportsPointerInteractionSelector]) {
    supportsPointerInteraction =
        performBoolSelector(XCUIDevice.sharedDevice, supportsPointerInteractionSelector);
  }
  XCTSkipUnless(supportsPointerInteraction, "Device does not support pointer interaction.");
  XCUIApplication* app = [[XCUIApplication alloc] init];
  app.launchArguments = @[ @"--pointer-events" ];
  [app launch];

  NSPredicate* predicateToFindFlutterView =
      [NSPredicate predicateWithBlock:^BOOL(id _Nullable evaluatedObject,
                                            NSDictionary<NSString*, id>* _Nullable bindings) {
        XCUIElement* element = evaluatedObject;
        return [element.identifier hasPrefix:@"flutter_view"];
      }];
  XCUIElement* flutterView = [[app descendantsMatchingType:XCUIElementTypeAny]
      elementMatchingPredicate:predicateToFindFlutterView];
  if (![flutterView waitForExistenceWithTimeout:kSecondsToWaitForFlutterView]) {
    NSLog(@"%@", app.debugDescription);
    XCTFail(@"Failed due to not able to find any flutterView with %@ seconds",
            @(kSecondsToWaitForFlutterView));
  }

  XCTAssertNotNil(flutterView);

  SEL hover = @selector(hover);
  XCTAssertTrue([flutterView respondsToSelector:hover],
                @"If supportsPointerInteraction is true, this should be true too.");
  [flutterView performSelector:hover];
  [NSThread sleepForTimeInterval:1.0];
  XCTAssertTrue(
      [app.textFields[@"0,PointerChange.add,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a hover");
  [flutterView tap];
  XCTAssertTrue(
      [app.textFields[@"1,PointerChange.add,device=1,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a tap");
  XCTAssertTrue(
      [app.textFields[@"2,PointerChange.down,device=1,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.down event did not occur for a tap");
  XCTAssertTrue(
      [app.textFields[@"3,PointerChange.up,device=1,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.up event did not occur for a tap");
  XCTAssertTrue(
      [app.textFields[@"4,PointerChange.remove,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"The hover pointer was not removed after a tap");
  [flutterView performSelector:hover];
  XCTAssertTrue(
      [app.textFields[@"5,PointerChange.add,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a subsequent hover");
}
#pragma clang diagnostic pop

@end
