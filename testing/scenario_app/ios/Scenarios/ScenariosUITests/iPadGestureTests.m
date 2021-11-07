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

static int assertOneMessageAndGetSequenceNo(NSMutableDictionary* messages, NSString* message) {
  NSMutableArray* matchingMessages = [messages objectForKey:message];
  XCTAssertNotNil(matchingMessages, @"Did not receive \"%@\" message", message);
  XCTAssertEqual([matchingMessages count], 1, @"More than one \"%@\" message", message);
  return [[matchingMessages firstObject] intValue];
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
  // On simulated right click, a hover also occurs, so the hover pointer is added
  XCTAssertTrue(
      [app.textFields[@"3,PointerChange.add,device=1,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a right-click's hover pointer");
  [NSThread sleepForTimeInterval:5.0];  // The hover pointer is removed after ~3.5 seconds, this
                                        // ensures all events have been received
  // The hover events are interspersed with the right-click events in a varying order
  // Ensure the individual orderings are respected without hardcoding the absolute sequence
  NSMutableDictionary* messages = [NSMutableDictionary dictionary];
  for (XCUIElement* element in [app.textFields allElementsBoundByIndex]) {
    NSString* rawMessage = element.value;
    NSUInteger commaIndex = [rawMessage rangeOfString:@","].location;
    NSInteger messageSequenceNo =
        [[rawMessage substringWithRange:NSMakeRange(0, commaIndex)] integerValue];
    NSString* message = [rawMessage
        substringWithRange:NSMakeRange(commaIndex + 1, [rawMessage length] - (commaIndex + 1))];
    NSMutableArray* messageSequenceNoList = [messages objectForKey:message];
    if (messageSequenceNoList == nil) {
      messageSequenceNoList = [[NSMutableArray alloc] init];
      [messages setObject:messageSequenceNoList forKey:message];
    }
    [messageSequenceNoList addObject:[NSNumber numberWithInteger:messageSequenceNo]];
  }
  // The number of hover events is not consistent
  NSMutableArray* hoverSeqNos = [messages objectForKey:@"PointerChange.hover,device=1,buttons=0"];
  int hoverRemovedSeqNo =
      assertOneMessageAndGetSequenceNo(messages, @"PointerChange.remove,device=1,buttons=0");
  // Right click should have buttons = 2
  int rightClickAddedSeqNo, rightClickDownSeqNo, rightClickUpSeqNo;
  if ([messages objectForKey:@"PointerChange.add,device=2,buttons=0"] == nil) {
    // Sometimes the tap pointer has the same device as the right-click (the UITouch is reused)
    rightClickAddedSeqNo = 0;
    rightClickDownSeqNo =
        assertOneMessageAndGetSequenceNo(messages, @"PointerChange.down,device=0,buttons=2");
    rightClickUpSeqNo =
        assertOneMessageAndGetSequenceNo(messages, @"PointerChange.up,device=0,buttons=2");
  } else {
    rightClickAddedSeqNo =
        assertOneMessageAndGetSequenceNo(messages, @"PointerChange.add,device=2,buttons=0");
    rightClickDownSeqNo =
        assertOneMessageAndGetSequenceNo(messages, @"PointerChange.down,device=2,buttons=2");
    rightClickUpSeqNo =
        assertOneMessageAndGetSequenceNo(messages, @"PointerChange.up,device=2,buttons=2");
  }
  XCTAssertGreaterThan(rightClickDownSeqNo, rightClickAddedSeqNo,
                       @"Right-click pointer was down before add");
  XCTAssertGreaterThan(rightClickUpSeqNo, rightClickDownSeqNo,
                       @"Right-click pointer was up before down");
  XCTAssertGreaterThan([[hoverSeqNos firstObject] intValue], 3,
                       @"Right-click pointer caused hover before hover pointer add");
  XCTAssertGreaterThan(hoverRemovedSeqNo, [[hoverSeqNos lastObject] intValue],
                       @"Right-click pointer's hover pointer had hover event after it was removed");
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
  XCTAssertTrue(
      [app.textFields[@"0,PointerChange.add,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.add event did not occur for a hover");
  XCTAssertTrue(
      [app.textFields[@"1,PointerChange.hover,device=0,buttons=0"] waitForExistenceWithTimeout:1],
      @"PointerChange.hover event did not occur for a hover");
  // The number of hover events fired is not always the same
  int i = 2;
  while (
      [app.textFields[[NSString stringWithFormat:@"%d,PointerChange.hover,device=0,buttons=0", i]]
          waitForExistenceWithTimeout:1]) {
    i++;
  }
  NSString* removeMessage =
      [NSString stringWithFormat:@"%d,PointerChange.remove,device=0,buttons=0", i];
  XCTAssertTrue([app.textFields[removeMessage] waitForExistenceWithTimeout:1],
                @"PointerChange.remove event did not occur for a hover");
}
#pragma clang diagnostic pop

@end
