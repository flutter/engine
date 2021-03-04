// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyChannelHandler.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace flutter::testing {

namespace {
NSEvent* keyEvent(NSEventType type,
                  NSEventModifierFlags modifierFlags,
                  NSString* characters,
                  NSString* charactersIgnoringModifiers,
                  BOOL isARepeat,
                  unsigned short keyCode) {
  return [NSEvent keyEventWithType:type
                          location:NSZeroPoint
                     modifierFlags:modifierFlags
                         timestamp:0
                      windowNumber:0
                           context:nil
                        characters:characters
       charactersIgnoringModifiers:charactersIgnoringModifiers
                         isARepeat:isARepeat
                           keyCode:keyCode];
}
}  // namespace

TEST(FlutterKeyChannelHandlerUnittests, BasicKeyEvent) {
  __block NSMutableArray<id>* messages = [[NSMutableArray<id> alloc] init];
  __block BOOL next_response = TRUE;
  __block NSMutableArray<NSNumber*>* responses = [[NSMutableArray<NSNumber*> alloc] init];

  id mockKeyEventChannel = OCMStrictClassMock([FlutterBasicMessageChannel class]);
  OCMStub([mockKeyEventChannel sendMessage:[OCMArg any] reply:[OCMArg any]])
      .andDo((^(NSInvocation* invocation) {
        [invocation retainArguments];
        NSDictionary* message;
        [invocation getArgument:&message atIndex:2];
        [messages addObject:message];

        FlutterReply callback;
        [invocation getArgument:&callback atIndex:3];
        NSDictionary* keyMessage = @{
          @"handled" : @(next_response),
        };
        callback(keyMessage);
      }));

  // Key down
  FlutterKeyChannelHandler* handler =
      [[FlutterKeyChannelHandler alloc] initWithChannel:mockKeyEventChannel];
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", FALSE, 0)
                ofType:@"keydown"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keydown");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 0);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x100);
  EXPECT_EQ([[messages lastObject][@"characters"] UTF8String], "a");
  EXPECT_EQ([[messages lastObject][@"charactersIgnoringModifiers"] UTF8String], "a");

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], TRUE);

  [messages removeAllObjects];
  [responses removeAllObjects];

  // Key up
  next_response = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x100, @"a", @"a", FALSE, 0)
                ofType:@"keyup"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keyup");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 0);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x100);
  EXPECT_EQ([[messages lastObject][@"characters"] UTF8String], "a");
  EXPECT_EQ([[messages lastObject][@"charactersIgnoringModifiers"] UTF8String], "a");

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], FALSE);

  [messages removeAllObjects];
  [responses removeAllObjects];

  // LShift down
  next_response = TRUE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, 56)
                ofType:@"keydown"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keydown");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 56);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x20102);

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], TRUE);

  [messages removeAllObjects];
  [responses removeAllObjects];

  // RShift down
  next_response = false;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20106, @"", @"", FALSE, 60)
                ofType:@"keydown"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keydown");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 60);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x20106);

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], FALSE);

  [messages removeAllObjects];
  [responses removeAllObjects];

  // LShift up
  next_response = false;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20104, @"", @"", FALSE, 56)
                ofType:@"keyup"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keyup");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 56);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x20104);

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], FALSE);

  [messages removeAllObjects];
  [responses removeAllObjects];

  // RShift up
  next_response = false;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, 60)
                ofType:@"keyup"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keyup");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 60);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x100);

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], FALSE);

  [messages removeAllObjects];
  [responses removeAllObjects];
}

TEST(FlutterKeyChannelHandlerUnittests, EmptyResponseIsTakenAsHandled) {
  __block NSMutableArray<id>* messages = [[NSMutableArray<id> alloc] init];
  __block NSMutableArray<NSNumber*>* responses = [[NSMutableArray<NSNumber*> alloc] init];

  id mockKeyEventChannel = OCMStrictClassMock([FlutterBasicMessageChannel class]);
  OCMStub([mockKeyEventChannel sendMessage:[OCMArg any] reply:[OCMArg any]])
      .andDo((^(NSInvocation* invocation) {
        [invocation retainArguments];
        NSDictionary* message;
        [invocation getArgument:&message atIndex:2];
        [messages addObject:message];

        FlutterReply callback;
        [invocation getArgument:&callback atIndex:3];
        callback(nullptr);
      }));

  FlutterKeyChannelHandler* handler =
      [[FlutterKeyChannelHandler alloc] initWithChannel:mockKeyEventChannel];
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x100, @"a", @"a", FALSE, 0)
                ofType:@"keydown"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_STREQ([[messages lastObject][@"keymap"] UTF8String], "macos");
  EXPECT_STREQ([[messages lastObject][@"type"] UTF8String], "keydown");
  EXPECT_EQ([[messages lastObject][@"keyCode"] intValue], 0);
  EXPECT_EQ([[messages lastObject][@"modifiers"] intValue], 0x100);
  EXPECT_EQ([[messages lastObject][@"characters"] UTF8String], "a");
  EXPECT_EQ([[messages lastObject][@"charactersIgnoringModifiers"] UTF8String], "a");

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], TRUE);
}

}  // namespace flutter::testing
