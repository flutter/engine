// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyChannelHandler.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace flutter::testing {

TEST(FlutterKeyChannelHandlerUnittests, BasicKeyEvent) {
  __block NSMutableArray<id>* messages = [[NSMutableArray<id> alloc] init];
  BOOL next_response = TRUE;
  __block NSMutableArray<NSNumber*>* responses = [[NSMutableArray<NSNumber*> alloc] init];

  id mockKeyEventChannel = OCMStrictClassMock([FlutterBasicMessageChannel class]);
  OCMStub([mockKeyEventChannel sendMessage:[OCMArg any] reply:[OCMArg any]]).andDo((
    ^(NSInvocation *invocation) {
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

  FlutterKeyChannelHandler* handler =
      [[FlutterKeyChannelHandler alloc] initWithChannel:mockKeyEventChannel];
  [handler handleEvent:[NSEvent keyEventWithType:NSKeyDown
                                              location:NSZeroPoint
                                         modifierFlags:0
                                             timestamp:0
                                          windowNumber:0
                                               context:nil
                                            characters:@"A"
                           charactersIgnoringModifiers:@"a"
                                             isARepeat:TRUE
                                               keyCode:0x60]
                ofType:@"keydown"
              callback:^(BOOL handled){
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([messages count], 1u);
  EXPECT_EQ([[messages firstObject][@"keyCode"] intValue], 0x60);
  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses firstObject] boolValue], TRUE);
}

}  // namespace flutter::testing
