// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#include <memory>
#include <vector>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyChannelHandler.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace flutter::testing {

TEST(FlutterKeyChannelHandlerUnittests, BasicKeyEvent) {
  __block std::vector<uint64_t> calls;

  id keyEventChannel = OCMClassMock([FlutterBasicMessageChannel class]);
  FlutterKeyChannelHandler* handler = [[FlutterKeyChannelHandler alloc]
      initWithChannel: keyEventChannel];
  [handler handleEvent:[NSEvent keyEventWithType:NSKeyDown
                                              location:NSZeroPoint
                                         modifierFlags:0
                                             timestamp:0
                                          windowNumber:0
                                               context:nil
                                            characters:@"a"
                           charactersIgnoringModifiers:@"a"
                                             isARepeat:FALSE
                                               keyCode:0x00000000]
                ofType:@"keydown"
              callback:^(BOOL handled){
              }];

  EXPECT_TRUE(calls.size() == 1);
}

}  // namespace flutter::testing
