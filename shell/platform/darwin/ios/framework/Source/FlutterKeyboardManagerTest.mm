// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>
#import <UIKit/UIKit.h>
#import <XCTest/XCTest.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterFakeKeyEvents.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterKeyboardManager.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterUIPressProxy.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterViewController_Internal.h"

FLUTTER_ASSERT_ARC;

using namespace flutter::testing;

// These tests were designed to run on iOS 13.4 or later.
API_AVAILABLE(ios(13.4))
@interface FlutterKeyboardManagerTest : XCTestCase
@end

@implementation FlutterKeyboardManagerTest

- (void)testNextResponderShouldThrowOnPressesEnded {
  // The nextResponder is a strict mock and hasn't stubbed pressesEnded.
  // An error will be thrown on pressesEnded.
  UIResponder* nextResponder = OCMStrictClassMock([UIResponder class]);
  OCMStub([nextResponder pressesBegan:OCMOCK_ANY withEvent:OCMOCK_ANY]);

  id mockEngine = OCMClassMock([FlutterEngine class]);
  FlutterViewController* viewController = [[FlutterViewController alloc] initWithEngine:mockEngine
                                                                                nibName:nil
                                                                                 bundle:nil];
  FlutterViewController* owner = OCMPartialMock(viewController);
  OCMStub([owner nextResponder]).andReturn(nextResponder);

  XCTAssertThrowsSpecificNamed([owner.nextResponder pressesEnded:[[NSSet alloc] init]
                                                       withEvent:[[UIPressesEvent alloc] init]],
                               NSException, NSInternalInconsistencyException);

  [mockEngine stopMocking];
}

- (void)testSinglePrimaryResponder {
  const UIKeyboardHIDUsage keyId = (UIKeyboardHIDUsage)0x50;

  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] init];
  id mockPrimaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  [manager addPrimaryResponder:mockPrimaryResponder];

  // Case: The responder reports that is has handled the event (callback returns YES), and the
  // manager should NOT call next action.
  [[mockPrimaryResponder expect] handlePress:OCMOCK_ANY
                                    callback:[OCMArg invokeBlockWithArgs:@YES, nil]];

  XCTestExpectation* expectationPrimaryHandled =
      [self expectationWithDescription:@"primary responder handled"];
  expectationPrimaryHandled.inverted = YES;  // Fail if the manager "next action" is called.
  [manager handlePress:keyDownEvent(keyId)
            nextAction:^() {
              [expectationPrimaryHandled fulfill];
              XCTFail();
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];

  // Case: The responder reports that is has NOT handled the event (callback returns NO), and the
  // manager SHOULD not call next action.
  [[mockPrimaryResponder expect] handlePress:OCMOCK_ANY
                                    callback:[OCMArg invokeBlockWithArgs:@NO, nil]];
  id expectationPrimaryNotHandled =
      [self expectationWithDescription:@"primary responder not handled"];
  [manager handlePress:keyUpEvent(keyId)
            nextAction:^() {
              [expectationPrimaryNotHandled fulfill];
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
}

- (void)testDoublePrimaryResponder {
  const UIKeyboardHIDUsage keyId = (UIKeyboardHIDUsage)0x50;

  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] init];
  id mockPrimaryResponder1 = OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  id mockPrimaryResponder2 = OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  [manager addPrimaryResponder:mockPrimaryResponder1];
  [manager addPrimaryResponder:mockPrimaryResponder2];

  // Case: Both responders report they have handled the event (callbacks returns YES), and the
  // manager should NOT call next action.
  [[mockPrimaryResponder1 expect] handlePress:OCMOCK_ANY
                                     callback:[OCMArg invokeBlockWithArgs:@YES, nil]];
  [[mockPrimaryResponder2 expect] handlePress:OCMOCK_ANY
                                     callback:[OCMArg invokeBlockWithArgs:@YES, nil]];

  XCTestExpectation* expectationBothPrimariesHandled =
      [self expectationWithDescription:@"both primary responders handled"];
  expectationBothPrimariesHandled.inverted = YES;  // Fail if the manager "next action" is called.
  [manager handlePress:keyDownEvent(keyId)
            nextAction:^() {
              [expectationBothPrimariesHandled fulfill];
              XCTFail();
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
  OCMVerifyAll(mockPrimaryResponder1);
  OCMVerifyAll(mockPrimaryResponder2);

  // Case: Only one responder reports it has handled the event (callback returns YES), and the
  // manager should NOT call next action.
  [[mockPrimaryResponder1 expect] handlePress:OCMOCK_ANY
                                     callback:[OCMArg invokeBlockWithArgs:@YES, nil]];
  [[mockPrimaryResponder2 expect] handlePress:OCMOCK_ANY
                                     callback:[OCMArg invokeBlockWithArgs:@NO, nil]];

  XCTestExpectation* expectationOnePrimaryHandled =
      [self expectationWithDescription:@"one primary responder handled"];
  expectationOnePrimaryHandled.inverted = YES;  // Fail if the manager "next action" is called.
  [manager handlePress:keyDownEvent(keyId)
            nextAction:^() {
              [expectationOnePrimaryHandled fulfill];
              XCTFail();
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
  OCMVerifyAll(mockPrimaryResponder1);
  OCMVerifyAll(mockPrimaryResponder2);

  // Case: Both responders report they have NOT handled the event (callbacks returns NO), and the
  // manager SHOULD not call next action.
  [[mockPrimaryResponder1 expect] handlePress:OCMOCK_ANY
                                     callback:[OCMArg invokeBlockWithArgs:@NO, nil]];
  [[mockPrimaryResponder2 expect] handlePress:OCMOCK_ANY
                                     callback:[OCMArg invokeBlockWithArgs:@NO, nil]];
  id expectationPrimariesNotHandled =
      [self expectationWithDescription:@"primary responders not handled"];
  [manager handlePress:keyUpEvent(keyId)
            nextAction:^() {
              [expectationPrimariesNotHandled fulfill];
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
  OCMVerifyAll(mockPrimaryResponder1);
  OCMVerifyAll(mockPrimaryResponder2);
}

- (void)testPrimaryResponderHandlesNotSecondaryResponder {
  const UIKeyboardHIDUsage keyId = (UIKeyboardHIDUsage)0x50;

  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] init];
  id mockPrimaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  id mockSecondaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeySecondaryResponder));
  [manager addPrimaryResponder:mockPrimaryResponder];
  [manager addSecondaryResponder:mockSecondaryResponder];

  // Primary responder responds TRUE. The event shouldn't be handled by
  // the secondary responder, and the manager should NOT call next action.
  [[mockPrimaryResponder expect] handlePress:OCMOCK_ANY
                                    callback:[OCMArg invokeBlockWithArgs:@YES, nil]];
  OCMReject([mockSecondaryResponder handlePress:OCMOCK_ANY]);

  XCTestExpectation* nextActionExpectation = [self expectationWithDescription:@"next action"];
  nextActionExpectation.inverted = YES;  // Fail if the manager "next action" is called.
  [manager handlePress:keyDownEvent(keyId)
            nextAction:^() {
              [nextActionExpectation fulfill];
              XCTFail();
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
  OCMVerifyAll(mockPrimaryResponder);
  OCMVerifyAll(mockSecondaryResponder);
}

- (void)testSecondaryResponderHandles {
  const UIKeyboardHIDUsage keyId = (UIKeyboardHIDUsage)0x50;

  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] init];
  id mockPrimaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  id mockSecondaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeySecondaryResponder));
  [manager addPrimaryResponder:mockPrimaryResponder];
  [manager addSecondaryResponder:mockSecondaryResponder];

  // Primary responder responds TRUE. The event shouldn't be handled by
  // the secondary responder, and the manager should NOT call next action.
  [[mockPrimaryResponder expect] handlePress:OCMOCK_ANY
                                    callback:[OCMArg invokeBlockWithArgs:@NO, nil]];
  OCMExpect([mockSecondaryResponder handlePress:OCMOCK_ANY]).andReturn(YES);

  XCTestExpectation* nextActionExpectation = [self expectationWithDescription:@"next action"];
  nextActionExpectation.inverted = YES;  // Fail if the manager "next action" is called.
  [manager handlePress:keyDownEvent(keyId)
            nextAction:^() {
              [nextActionExpectation fulfill];
              XCTFail();
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
  OCMVerifyAll(mockPrimaryResponder);
  OCMVerifyAll(mockSecondaryResponder);
}

- (void)testPrimaryAndSecondaryResponderDoNotHandle {
  const UIKeyboardHIDUsage keyId = (UIKeyboardHIDUsage)0x50;

  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] init];
  id mockPrimaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  id mockSecondaryResponder = OCMStrictProtocolMock(@protocol(FlutterKeySecondaryResponder));
  [manager addPrimaryResponder:mockPrimaryResponder];
  [manager addSecondaryResponder:mockSecondaryResponder];

  // Primary responder responds TRUE. The event shouldn't be handled by
  // the secondary responder, and the manager should NOT call next action.
  [[mockPrimaryResponder expect] handlePress:OCMOCK_ANY
                                    callback:[OCMArg invokeBlockWithArgs:@NO, nil]];
  OCMExpect([mockSecondaryResponder handlePress:OCMOCK_ANY]).andReturn(NO);

  XCTestExpectation* nextActionExpectation = [self expectationWithDescription:@"next action"];
  [manager handlePress:keyDownEvent(keyId)
            nextAction:^() {
              [nextActionExpectation fulfill];
            }];
  [self waitForExpectationsWithTimeout:1.0 handler:nil];
  OCMVerifyAll(mockPrimaryResponder);
  OCMVerifyAll(mockSecondaryResponder);
}

- (void)testEventsProcessedSequentially {
  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] init];
  id<FlutterKeyPrimaryResponder> mockPrimaryResponder =
      OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
  [manager addPrimaryResponder:mockPrimaryResponder];

  const UIKeyboardHIDUsage keyId1 = (UIKeyboardHIDUsage)0x50;
  FlutterUIPressProxy* event1 = keyDownEvent(keyId1);
  id expectationEvent1Primary = [self expectationWithDescription:@"event1 primary responder"];
  OCMExpect([mockPrimaryResponder handlePress:event1 callback:OCMOCK_ANY])
      .andDo((^(NSInvocation* invocation) {
        [expectationEvent1Primary fulfill];
      }));

  const UIKeyboardHIDUsage keyId2 = (UIKeyboardHIDUsage)0x51;
  FlutterUIPressProxy* event2 = keyDownEvent(keyId2);
  id expectationEvent2Primary = [self expectationWithDescription:@"event2 primary responder"];
  OCMExpect([mockPrimaryResponder handlePress:event2 callback:OCMOCK_ANY])
      .andDo((^(NSInvocation* invocation) {
        [expectationEvent2Primary fulfill];
      }));

  id expectationEvent1Action = [self expectationWithDescription:@"event1 action"];
  [manager handlePress:event1
            nextAction:^() {
              [expectationEvent1Action fulfill];
            }];

  id expectationEvent2Action = [self expectationWithDescription:@"event2 action"];
  [manager handlePress:event2
            nextAction:^() {
              [expectationEvent2Action fulfill];
            }];

  [self waitForExpectations:@[
    expectationEvent1Primary, expectationEvent1Action, expectationEvent2Primary,
    expectationEvent2Action
  ]
                    timeout:1.0
               enforceOrder:YES];
  OCMVerifyAll(mockPrimaryResponder);
}

@end
