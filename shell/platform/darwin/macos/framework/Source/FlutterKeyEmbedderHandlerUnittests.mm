// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyEmbedderHandler.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

// A wrap to convert FlutterKeyEvent to a ObjC class.
@interface TestKeyEvent : NSObject {
 @public
  FlutterKeyEvent* data;
}
- (nonnull instancetype)initWithEvent:(nonnull const FlutterKeyEvent*)event;
@end

@implementation TestKeyEvent
- (instancetype)initWithEvent:(const FlutterKeyEvent*)event {
  self = [super init];
  data = new FlutterKeyEvent(*event);
  if (event->character != nullptr) {
    size_t len = strlen(event->character);
    char* character = new char[len + 1];
    strcpy(character, event->character);
    data->character = character;
  }
  return self;
}

- (void)dealloc {
  if (data->character != nullptr)
    delete[] data->character;
  delete data;
}
@end

@interface Utils : NSObject
+ (nonnull NSEvent*)keyEventWithType:(NSEventType)type
                       modifierFlags:(NSEventModifierFlags)flags
                          characters:(NSString*)keys
         charactersIgnoringModifiers:(NSString*)ukeys
                           isARepeat:(BOOL)flag
                             keyCode:(unsigned short)code;
@end

@implementation Utils
+ (nonnull NSEvent*)keyEventWithType:(NSEventType)type
                       modifierFlags:(NSEventModifierFlags)flags
                          characters:(NSString*)keys
         charactersIgnoringModifiers:(NSString*)ukeys
                           isARepeat:(BOOL)flag
                             keyCode:(unsigned short)code {
  return [NSEvent keyEventWithType:type
                          location:NSZeroPoint
                     modifierFlags:flags
                         timestamp:0
                      windowNumber:0
                           context:nil
                        characters:keys
       charactersIgnoringModifiers:ukeys
                         isARepeat:flag
                           keyCode:code];
}
@end

namespace flutter::testing {

namespace {
constexpr uint64_t kPhysicalKeyA = 0x00070004;
// constexpr uint64_t kPhysicalControlLeft = 0x000700e0;
// constexpr uint64_t kPhysicalControlRight = 0x000700e4;
// constexpr uint64_t kPhysicalShiftLeft = 0x000700e1;
// constexpr uint64_t kPhysicalShiftRight = 0x000700e5;
// constexpr uint64_t kPhysicalKeyNumLock = 0x00070053;

constexpr uint64_t kLogicalKeyA = 0x00000061;
// constexpr uint64_t kLogicalControlLeft = 0x00300000105;
// constexpr uint64_t kLogicalControlRight = 0x00400000105;
// constexpr uint64_t kLogicalShiftLeft = 0x0030000010d;
// constexpr uint64_t kLogicalShiftRight = 0x0040000010d;
// constexpr uint64_t kLogicalKeyNumLock = 0x0000000010a;

}

// Test the most basic key events.
//
// Press, hold, and release key A on an US keyboard.
TEST(FlutterKeyEmbedderHandlerUnittests, BasicKeyEvent) {
  __block BOOL next_handled = TRUE;
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block NSMutableArray<NSNumber*>* responses = [[NSMutableArray<NSNumber*> alloc] init];
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event]];
        printf("callback %d handled %d\n", !!callback, next_handled);
        if (callback != nullptr) {
          callback(next_handled, user_data);
        }
      }];

  next_handled = TRUE;
  [handler handleEvent:[Utils keyEventWithType:NSKeyDown
                                          modifierFlags:0
                                            characters:@"a"
                           charactersIgnoringModifiers:@"a"
                                             isARepeat:FALSE
                                               keyCode:0]
                ofType:@"keydown"
              callback:^(BOOL handled) {
                [responses addObject:@(handled)];
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ([responses count], 1u);
  EXPECT_EQ([[responses lastObject] boolValue], TRUE);
}

}  // namespace flutter::testing
