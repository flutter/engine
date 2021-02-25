// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#include <memory>
#include <vector>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyEmbedderHandler.h"
#include "flutter/shell/platform/embedder/embedder.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace flutter::testing {

// @implementation FlutterViewController

// - (void)addKeyResponder:(nonnull FlutterIntermediateKeyResponder*)responder;

// /**
//  * Removes an intermediate responder for keyboard events.
//  */
// - (void)removeKeyResponder:(nonnull FlutterIntermediateKeyResponder*)responder;

// - (void)handleNSKeyEvent:(NSEvent*)event reply:(FlutterReply)callback;

// /**
//  * Send a FlutterKeyEvent to the framework using embedder API.
//  *
//  * Called by FlutterKeyboardPlugin.
//  */
// - (void)dispatchFlutterKeyEvent:(const FlutterKeyEvent&)event;

// /**
//  * Initializes this FlutterViewController with the specified `FlutterEngine`.
//  *
//  * The initialized viewcontroller will attach itself to the engine as part of this process.
//  *
//  * @param engine The `FlutterEngine` instance to attach to. Cannot be nil.
//  * @param nibName The NIB name to initialize this controller with.
//  * @param nibBundle The NIB bundle.
//  */
// - (nonnull instancetype)initWithEngine:(nonnull FlutterEngine*)engine
//                                nibName:(nullable NSString*)nibName
//                                 bundle:(nullable NSBundle*)nibBundle NS_DESIGNATED_INITIALIZER;
// @end

TEST(FlutterKeybaordPluginUnitTests, TestTextureResolution) {
  __block std::vector<uint64_t> calls;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        calls.push_back(event.physical);
      }];
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
