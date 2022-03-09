// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

typedef BOOL (^BoolGetter)();
typedef void (^AsyncKeyCallback)(BOOL handled);
typedef void (^AsyncKeyCallbackHandler)(AsyncKeyCallback callback);

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

// id<FlutterKeyPrimaryResponder> mockPrimaryResponder(KeyCallbackSetter callbackSetter) {
//   id<FlutterKeyPrimaryResponder> mock =
//       OCMStrictProtocolMock(@protocol(FlutterKeyPrimaryResponder));
//   OCMStub([mock handleEvent:[OCMArg any] callback:[OCMArg any]])
//       .andDo((^(NSInvocation* invocation) {
//         FlutterAsyncKeyCallback callback;
//         [invocation getArgument:&callback atIndex:3];
//         callbackSetter(callback);
//       }));
//   return mock;
// }

// id<FlutterKeySecondaryResponder> mockSecondaryResponder(BoolGetter resultGetter) {
//   id<FlutterKeySecondaryResponder> mock =
//       OCMStrictProtocolMock(@protocol(FlutterKeySecondaryResponder));
//   OCMStub([mock handleKeyEvent:[OCMArg any]]).andDo((^(NSInvocation* invocation) {
//     BOOL result = resultGetter();
//     [invocation setReturnValue:&result];
//   }));
//   return mock;
// }

}  // namespace

@interface KeyboardTester : NSObject
- (nonnull instancetype)init;
- (void)respondEmbedderCallsWith:(BOOL)response;
- (void)recordEmbedderCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage;
- (void)respondChannelCallsWith:(BOOL)response;
- (void)recordChannelCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage;

@property(nonatomic) FlutterKeyboardManager* manager;
@property(nonatomic) NSResponder* nextResponder;

#pragma mark - Private

- (void)handleEmbedderEvent:(const FlutterKeyEvent&)event
                   callback:(nullable FlutterKeyEventCallback)callback
                   userData:(nullable void*)userData;

- (void)handleChannelMessage:(NSString*)channel
                     message:(NSData* _Nullable)message
                 binaryReply:(FlutterBinaryReply _Nullable)callback;

- (BOOL)handleTextInputKeyEvent:(NSEvent*)event;
@end

@implementation KeyboardTester {
  AsyncKeyCallbackHandler _embedderHandler;
  AsyncKeyCallbackHandler _channelHandler;
}

- (nonnull instancetype)init {
  self = [super init];
  if (self == nil) {
    return nil;
  }

  _nextResponder = OCMClassMock([NSResponder class]);
  [self respondChannelCallsWith:false];
  [self respondEmbedderCallsWith:false];

  id engineMock = OCMStrictClassMock([FlutterEngine class]);
  OCMStub(  // NOLINT(google-objc-avoid-throwing-exception)
      [engineMock binaryMessenger])
      .andReturn(engineMock);
  OCMStub([engineMock sendKeyEvent:FlutterKeyEvent {} callback:nil userData:nil])
      .ignoringNonObjectArgs()
      .andCall(self, @selector(handleEmbedderEvent:callback:userData:));
  OCMStub([engineMock sendOnChannel:@"flutter/keyevent"
                            message:[OCMArg any]
                        binaryReply:[OCMArg any]])
      .andCall(self, @selector(handleChannelMessage:message:binaryReply:));

  id viewDelegateMock = OCMStrictProtocolMock(@protocol(FlutterKeyboardViewDelegate));
  OCMStub([viewDelegateMock nextResponder]).andReturn(_nextResponder);
  OCMStub([viewDelegateMock onTextInputKeyEvent:[OCMArg any]])
      .andCall(self, @selector(handleTextInputKeyEvent:));

  _manager = [[FlutterKeyboardManager alloc] initWithEngine:engineMock
                                               viewDelegate:viewDelegateMock];
  return self;
}

- (void)respondEmbedderCallsWith:(BOOL)response {
  _embedderHandler = ^(AsyncKeyCallback callback) {
    if (callback != nil)
      callback(response);
  };
}

- (void)recordEmbedderCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage {
  _embedderHandler = ^(AsyncKeyCallback callback) {
    [storage addObject:callback];
  };
}

- (void)respondChannelCallsWith:(BOOL)response {
  _channelHandler = ^(AsyncKeyCallback callback) {
    if (callback != nil)
      callback(response);
  };
}

- (void)recordChannelCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage {
  _channelHandler = ^(AsyncKeyCallback callback) {
    [storage addObject:callback];
  };
}

#pragma mark - Private

- (void)handleEmbedderEvent:(const FlutterKeyEvent&)event
                   callback:(nullable FlutterKeyEventCallback)callback
                   userData:(nullable void*)userData {
  _embedderHandler(^(BOOL handled) {
    callback(handled, userData);
  });
}

- (void)handleChannelMessage:(NSString*)channel
                     message:(NSData* _Nullable)message
                 binaryReply:(FlutterBinaryReply _Nullable)callback {
  NSDictionary* result = @{
    @"handled" : @false,
  };
  NSData* encodedKeyEvent = [[FlutterJSONMessageCodec sharedInstance] encode:result];
  callback(encodedKeyEvent);
}

- (BOOL)handleTextInputKeyEvent:(NSEvent*)event {
  return NO;
}

@end

@interface FlutterKeyboardManagerUnittestsObjC : NSObject
// - (bool)nextResponderShouldThrowOnKeyUp;
- (bool)singlePrimaryResponder;
// - (bool)doublePrimaryResponder;
// - (bool)singleSecondaryResponder;
// - (bool)emptyNextResponder;
@end

namespace flutter::testing {
// TEST(FlutterKeyboardManagerUnittests, NextResponderShouldThrowOnKeyUp) {
//   ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] nextResponderShouldThrowOnKeyUp]);
// }

TEST(FlutterKeyboardManagerUnittests, SinglePrimaryResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] singlePrimaryResponder]);
}

// TEST(FlutterKeyboardManagerUnittests, DoublePrimaryResponder) {
//   ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] doublePrimaryResponder]);
// }

// TEST(FlutterKeyboardManagerUnittests, SingleFinalResponder) {
//   ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] singleSecondaryResponder]);
// }

// TEST(FlutterKeyboardManagerUnittests, EmptyNextResponder) {
//   ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] emptyNextResponder]);
// }

}  // namespace flutter::testing

@implementation FlutterKeyboardManagerUnittestsObjC

// Verify that the nextResponder returned from mockOwnerWithDownOnlyNext()
// throws exception when keyUp is called.
- (bool)nextResponderShouldThrowOnKeyUp {
  NSResponder* owner = mockOwnerWithDownOnlyNext();
  @try {
    [owner.nextResponder keyUp:keyUpEvent(0x50)];
    return false;
  } @catch (...) {
    return true;
  }
}

- (bool)singlePrimaryResponder {
  KeyboardTester* tester = [[KeyboardTester alloc] init];
  NSMutableArray<FlutterAsyncKeyCallback>* embedderCallbacks =
      [NSMutableArray<FlutterAsyncKeyCallback> array];
  [tester recordEmbedderCallsTo:embedderCallbacks];

  // Case: The responder reports FALSE
  [tester.manager handleEvent:keyDownEvent(0x50)];
  EXPECT_EQ([embedderCallbacks count], 1u);
  embedderCallbacks[0](FALSE);
  OCMVerify([tester.nextResponder keyDown:checkKeyDownEvent(0x50)]);
  [embedderCallbacks removeAllObjects];

  // Case: The responder reports TRUE
  [tester.manager handleEvent:keyUpEvent(0x50)];
  EXPECT_EQ([embedderCallbacks count], 1u);
  embedderCallbacks[0](TRUE);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.

  return true;
}

//- (bool)doublePrimaryResponder {
//  NSResponder* owner = mockOwnerWithDownOnlyNext();
//  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];
//
//  __block NSMutableArray<FlutterAsyncKeyCallback>* callbacks1 =
//      [NSMutableArray<FlutterAsyncKeyCallback> array];
//  [manager addPrimaryResponder:mockPrimaryResponder(
//                                   ^(FlutterAsyncKeyCallback callback) {
//                                     [callbacks1 addObject:callback];
//                                   })];
//
//  __block NSMutableArray<FlutterAsyncKeyCallback>* callbacks2 =
//      [NSMutableArray<FlutterAsyncKeyCallback> array];
//  [manager addPrimaryResponder:mockPrimaryResponder(
//                                   ^(FlutterAsyncKeyCallback callback) {
//                                     [callbacks2 addObject:callback];
//                                   })];
//
//  // Case: Both responder report TRUE.
//  [manager handleEvent:keyUpEvent(0x50)];
//  EXPECT_EQ([callbacks1 count], 1u);
//  EXPECT_EQ([callbacks2 count], 1u);
//  callbacks1[0](TRUE);
//  callbacks2[0](TRUE);
//  EXPECT_EQ([callbacks1 count], 1u);
//  EXPECT_EQ([callbacks2 count], 1u);
//  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
//  [callbacks1 removeAllObjects];
//  [callbacks2 removeAllObjects];
//
//  // Case: One responder reports TRUE.
//  [manager handleEvent:keyUpEvent(0x50)];
//  EXPECT_EQ([callbacks1 count], 1u);
//  EXPECT_EQ([callbacks2 count], 1u);
//  callbacks1[0](FALSE);
//  callbacks2[0](TRUE);
//  EXPECT_EQ([callbacks1 count], 1u);
//  EXPECT_EQ([callbacks2 count], 1u);
//  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
//  [callbacks1 removeAllObjects];
//  [callbacks2 removeAllObjects];
//
//  // Case: Both responders report FALSE.
//  [manager handleEvent:keyDownEvent(0x50)];
//  EXPECT_EQ([callbacks1 count], 1u);
//  EXPECT_EQ([callbacks2 count], 1u);
//  callbacks1[0](FALSE);
//  callbacks2[0](FALSE);
//  EXPECT_EQ([callbacks1 count], 1u);
//  EXPECT_EQ([callbacks2 count], 1u);
//  OCMVerify([owner.nextResponder keyDown:checkKeyDownEvent(0x50)]);
//  [callbacks1 removeAllObjects];
//  [callbacks2 removeAllObjects];
//
//  return true;
//}
//
//- (bool)singleSecondaryResponder {
//  NSResponder* owner = mockOwnerWithDownOnlyNext();
//  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];
//
//  __block NSMutableArray<FlutterAsyncKeyCallback>* callbacks =
//      [NSMutableArray<FlutterAsyncKeyCallback> array];
//  [manager addPrimaryResponder:mockPrimaryResponder(
//                                   ^(FlutterAsyncKeyCallback callback) {
//                                     [callbacks addObject:callback];
//                                   })];
//
//  __block BOOL nextResponse;
//  [manager addSecondaryResponder:mockSecondaryResponder(^() {
//             return nextResponse;
//           })];
//
//  // Case: Primary responder responds TRUE. The event shouldn't be handled by
//  // the secondary responder.
//  nextResponse = FALSE;
//  [manager handleEvent:keyUpEvent(0x50)];
//  EXPECT_EQ([callbacks count], 1u);
//  callbacks[0](TRUE);
//  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
//  [callbacks removeAllObjects];
//
//  // Case: Primary responder responds FALSE. The secondary responder returns
//  // TRUE.
//  nextResponse = TRUE;
//  [manager handleEvent:keyUpEvent(0x50)];
//  EXPECT_EQ([callbacks count], 1u);
//  callbacks[0](FALSE);
//  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
//  [callbacks removeAllObjects];
//
//  // Case: Primary responder responds FALSE. The secondary responder returns FALSE.
//  nextResponse = FALSE;
//  [manager handleEvent:keyDownEvent(0x50)];
//  EXPECT_EQ([callbacks count], 1u);
//  callbacks[0](FALSE);
//  OCMVerify([owner.nextResponder keyDown:checkKeyDownEvent(0x50)]);
//  [callbacks removeAllObjects];
//
//  return true;
//}
//
//- (bool)emptyNextResponder {
//  NSResponder* owner = OCMStrictClassMock([NSResponder class]);
//  OCMStub([owner nextResponder]).andReturn(nil);
//
//  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];
//
//  [manager addPrimaryResponder:mockPrimaryResponder(
//                                   ^(FlutterAsyncKeyCallback callback) {
//                                     callback(FALSE);
//                                   })];
//  // Passes if no error is thrown.
//  return true;
//}

@end
