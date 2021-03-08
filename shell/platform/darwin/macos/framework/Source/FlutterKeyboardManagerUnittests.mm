// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

@interface FlutterKeyboardManagerUnittestsObjC : NSObject
- (bool)nextResponderShouldThrowOnKeyUp;
- (bool)singleAsyncHandler;
- (bool)doubleAsyncHandlers;
- (bool)singleFinalResponder;
- (bool)emptyNextResponder;
@end

namespace flutter::testing {

namespace {

NSEvent* keyDownEvent(unsigned short keyCode) {
  return [NSEvent keyEventWithType:NSEventTypeKeyDown
                          location:NSZeroPoint
                     modifierFlags:0x100
                         timestamp:0
                      windowNumber:0
                           context:nil
                        characters:@""
       charactersIgnoringModifiers:@""
                         isARepeat:NO
                           keyCode:keyCode];
}

NSEvent* keyUpEvent(unsigned short keyCode) {
  return [NSEvent keyEventWithType:NSEventTypeKeyUp
                          location:NSZeroPoint
                     modifierFlags:0x100
                         timestamp:0
                      windowNumber:0
                           context:nil
                        characters:@""
       charactersIgnoringModifiers:@""
                         isARepeat:NO
                           keyCode:keyCode];
}

id checkKeyDownEvent(unsigned short keyCode) {
  return [OCMArg checkWithBlock:^BOOL(id value) {
    if (![value isKindOfClass:[NSEvent class]]) {
      return NO;
    }
    NSEvent* event = value;
    return event.keyCode == keyCode;
  }];
}

NSResponder* mockOwnerWithDownOnlyNext() {
  NSResponder* nextResponder = OCMStrictClassMock([NSResponder class]);
  OCMStub([nextResponder keyDown:[OCMArg any]]).andDo(nil);
  // The nextResponder is a strict mock and hasn't stubbed keyUp.
  // An error will be thrown on keyUp.

  NSResponder* owner = OCMStrictClassMock([NSResponder class]);
  OCMStub([owner nextResponder]).andReturn(nextResponder);
  return owner;
}

typedef void (^KeyCallbackHandler)(FlutterKeyHandlerCallback callback);
typedef BOOL (^BoolGetter)();

id<FlutterKeyHandler> mockAsyncKeyHandler(KeyCallbackHandler handler) {
  id<FlutterKeyHandler> mock = OCMStrictProtocolMock(@protocol(FlutterKeyHandler));
  OCMStub([mock handleEvent:[OCMArg any] callback:[OCMArg any]])
      .andDo((^(NSInvocation* invocation) {
        FlutterKeyHandlerCallback callback;
        [invocation getArgument:&callback atIndex:3];
        handler(callback);
      }));
  return mock;
}

id<FlutterKeyFinalResponder> mockFinalResponder(BoolGetter resultGetter) {
  id<FlutterKeyFinalResponder> mock = OCMStrictProtocolMock(@protocol(FlutterKeyFinalResponder));
  OCMStub([mock handleKeyEvent:[OCMArg any]])
      .andDo((^(NSInvocation* invocation) {
        BOOL result = resultGetter();
        [invocation setReturnValue:&result];
      }));
  return mock;
}

}  // namespace

TEST(FlutterKeyboardManagerUnittests, NextResponderShouldThrowOnKeyUp) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] nextResponderShouldThrowOnKeyUp]);
}

TEST(FlutterKeyboardManagerUnittests, SingleAsyncHandler) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] singleAsyncHandler]);
}

TEST(FlutterKeyboardManagerUnittests, DoubleAsyncHandlers) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] doubleAsyncHandlers]);
}

TEST(FlutterKeyboardManagerUnittests, SingleFinalResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] singleFinalResponder]);
}

TEST(FlutterKeyboardManagerUnittests, EmptyNextResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] emptyNextResponder]);
}

}  // namespace flutter::testing

@implementation FlutterKeyboardManagerUnittestsObjC

// Verify that the nextResponder returned from mockOwnerWithDownOnlyNext()
// throws exception when keyUp is called.
- (bool)nextResponderShouldThrowOnKeyUp {
  NSResponder* owner = flutter::testing::mockOwnerWithDownOnlyNext();
  @try {
    [owner.nextResponder keyUp:flutter::testing::keyUpEvent(0x50)];
    return false;
  } @catch (...) {
    return true;
  }
}

- (bool)singleAsyncHandler {
  NSResponder* owner = flutter::testing::mockOwnerWithDownOnlyNext();
  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];

  __block NSMutableArray<FlutterKeyHandlerCallback>* callbacks =
      [NSMutableArray<FlutterKeyHandlerCallback> array];
  [manager addHandler:flutter::testing::mockAsyncKeyHandler(^(FlutterKeyHandlerCallback callback) {
             [callbacks addObject:callback];
           })];

  // Case: The handler reports FALSE
  [manager handleEvent:flutter::testing::keyDownEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](FALSE);
  OCMVerify([owner.nextResponder keyDown:flutter::testing::checkKeyDownEvent(0x50)]);
  [callbacks removeAllObjects];

  // Case: The handler reports TRUE
  [manager handleEvent:flutter::testing::keyUpEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](TRUE);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.

  return true;
}

- (bool)doubleAsyncHandlers {
  NSResponder* owner = flutter::testing::mockOwnerWithDownOnlyNext();
  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];

  __block NSMutableArray<FlutterKeyHandlerCallback>* callbacks1 =
      [NSMutableArray<FlutterKeyHandlerCallback> array];
  [manager addHandler:flutter::testing::mockAsyncKeyHandler(^(FlutterKeyHandlerCallback callback) {
             [callbacks1 addObject:callback];
           })];

  __block NSMutableArray<FlutterKeyHandlerCallback>* callbacks2 =
      [NSMutableArray<FlutterKeyHandlerCallback> array];
  [manager addHandler:flutter::testing::mockAsyncKeyHandler(^(FlutterKeyHandlerCallback callback) {
             [callbacks2 addObject:callback];
           })];

  // Case: Both handler report TRUE.
  [manager handleEvent:flutter::testing::keyUpEvent(0x50)];
  EXPECT_EQ([callbacks1 count], 1u);
  EXPECT_EQ([callbacks2 count], 1u);
  callbacks1[0](TRUE);
  callbacks2[0](TRUE);
  EXPECT_EQ([callbacks1 count], 1u);
  EXPECT_EQ([callbacks2 count], 1u);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [callbacks1 removeAllObjects];
  [callbacks2 removeAllObjects];

  // Case: One handler reports TRUE.
  [manager handleEvent:flutter::testing::keyUpEvent(0x50)];
  EXPECT_EQ([callbacks1 count], 1u);
  EXPECT_EQ([callbacks2 count], 1u);
  callbacks1[0](FALSE);
  callbacks2[0](TRUE);
  EXPECT_EQ([callbacks1 count], 1u);
  EXPECT_EQ([callbacks2 count], 1u);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [callbacks1 removeAllObjects];
  [callbacks2 removeAllObjects];

  // Case: Both handlers report FALSE.
  [manager handleEvent:flutter::testing::keyDownEvent(0x50)];
  EXPECT_EQ([callbacks1 count], 1u);
  EXPECT_EQ([callbacks2 count], 1u);
  callbacks1[0](FALSE);
  callbacks2[0](FALSE);
  EXPECT_EQ([callbacks1 count], 1u);
  EXPECT_EQ([callbacks2 count], 1u);
  OCMVerify([owner.nextResponder keyDown:flutter::testing::checkKeyDownEvent(0x50)]);
  [callbacks1 removeAllObjects];
  [callbacks2 removeAllObjects];

  return true;
}

- (bool)singleFinalResponder {
  NSResponder* owner = flutter::testing::mockOwnerWithDownOnlyNext();
  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];

  __block NSMutableArray<FlutterKeyHandlerCallback>* callbacks =
      [NSMutableArray<FlutterKeyHandlerCallback> array];
  [manager addHandler:flutter::testing::mockAsyncKeyHandler(^(FlutterKeyHandlerCallback callback) {
             [callbacks addObject:callback];
           })];

  __block BOOL nextResponse;
  [manager addAdditionalHandler:flutter::testing::mockFinalResponder(^() {
             return nextResponse;
           })];

  // Case: Handler responds TRUE. The event shouldn't be handled by the final
  // responder.
  nextResponse = FALSE;
  [manager handleEvent:flutter::testing::keyUpEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](TRUE);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [callbacks removeAllObjects];

  // Case: Handler responds FALSE. The final responder returns TRUE.
  nextResponse = TRUE;
  [manager handleEvent:flutter::testing::keyUpEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](FALSE);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [callbacks removeAllObjects];

  // Case: Handler responds FALSE. The final responder returns FALSE.
  nextResponse = FALSE;
  [manager handleEvent:flutter::testing::keyDownEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](FALSE);
  OCMVerify([owner.nextResponder keyDown:flutter::testing::checkKeyDownEvent(0x50)]);
  [callbacks removeAllObjects];

  return true;
}

- (bool)emptyNextResponder {
  NSResponder* owner = OCMStrictClassMock([NSResponder class]);
  OCMStub([owner nextResponder]).andReturn(nil);

  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];

  [manager addHandler:flutter::testing::mockAsyncKeyHandler(^(FlutterKeyHandlerCallback callback) {
    callback(FALSE);
  })];
  // Passes if no error is thrown.
  return true;
}


@end
