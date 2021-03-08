// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace flutter::testing {

namespace {

NSResponder* mockResponder() {
  NSResponder* mock = OCMStrictClassMock([NSResponder class]);
  OCMStub([mock keyDown:[OCMArg any]]).andDo(nil);
  OCMStub([mock keyUp:[OCMArg any]]).andDo(nil);
  OCMStub([mock flagsChanged:[OCMArg any]]).andDo(nil);
  return mock;
}

typedef void (^KeyCallbackHandler)(FlutterKeyHandlerCallback callback);

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

// NSResponder* mockKeyHandler() {
//   NSResponder* mock = OCMStrictClassMock([NSResponder class]);
//   OCMStub([mock keyDown:[OCMArg any]]).andDo(nil);
//   OCMStub([mock keyUp:[OCMArg any]]).andDo(nil);
//   OCMStub([mock flagsChanged:[OCMArg any]]).andDo(nil);
//   return mock;
// }

} // namespace

TEST(FlutterKeyboardManagerUnittests, BasicKeyEvent) {
  NSResponder* owner = mockResponder();
  FlutterKeyboardManager* manager = [[FlutterKeyboardManager alloc] initWithOwner:owner];

  __block NSMutableArray<FlutterKeyHandlerCallback>* callbacks = [NSMutableArray<FlutterKeyHandlerCallback> array];
  [manager addHandler:mockAsyncKeyHandler(^(FlutterKeyHandlerCallback callback) {
    [callbacks addObject:callback];
  })];

  [manager handleEvent:keyDownEvent(0x50)];

  EXPECT_EQ([callbacks count], 1u);
}

}  // namespace flutter::testing
