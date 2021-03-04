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

namespace flutter::testing {

namespace {
constexpr uint64_t kKeyCodeKeyA = 0;
constexpr uint64_t kKeyCodeShiftLeft = 56;
constexpr uint64_t kKeyCodeShiftRight = 60;

constexpr uint64_t kPhysicalKeyA = 0x00070004;
// constexpr uint64_t kPhysicalControlLeft = 0x000700e0;
// constexpr uint64_t kPhysicalControlRight = 0x000700e4;
constexpr uint64_t kPhysicalShiftLeft = 0x000700e1;
constexpr uint64_t kPhysicalShiftRight = 0x000700e5;
// constexpr uint64_t kPhysicalKeyNumLock = 0x00070053;

constexpr uint64_t kLogicalKeyA = 0x00000061;
// constexpr uint64_t kLogicalControlLeft = 0x00300000105;
// constexpr uint64_t kLogicalControlRight = 0x00400000105;
constexpr uint64_t kLogicalShiftLeft = 0x0030000010d;
constexpr uint64_t kLogicalShiftRight = 0x0040000010d;
// constexpr uint64_t kLogicalKeyNumLock = 0x0000000010a;

typedef void (^ResponseCallback)(bool handled);

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

// Test the most basic key events.
//
// Press, hold, and release key A on an US keyboard.
TEST(FlutterKeyEmbedderHandlerUnittests, BasicKeyEvent) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block NSMutableArray<ResponseCallback>* callbacks =
      [[NSMutableArray<ResponseCallback> alloc] init];
  __block BOOL last_handled = TRUE;
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event]];
        if (callback != nullptr) {
          [callbacks addObject:^(bool handled) {
            callback(handled, user_data);
          }];
        }
      }];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", FALSE, 0)
              callback:^(BOOL handled) {
                last_handled = handled;
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_EQ([callbacks count], 1u);
  [callbacks lastObject](TRUE);
  EXPECT_EQ(last_handled, TRUE);

  [callbacks removeAllObjects];
  [events removeAllObjects];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", TRUE, kKeyCodeKeyA)
              callback:^(BOOL handled) {
                last_handled = handled;
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_EQ([callbacks count], 1u);
  [callbacks lastObject](TRUE);
  EXPECT_EQ(last_handled, TRUE);

  [callbacks removeAllObjects];
  [events removeAllObjects];

  last_handled = TRUE;
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x100, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled) {
                last_handled = handled;
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_EQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, TRUE);
  EXPECT_EQ([callbacks count], 1u);
  [callbacks lastObject](FALSE);
  EXPECT_EQ(last_handled, FALSE);

  [callbacks removeAllObjects];
  [events removeAllObjects];
}

// Press L shift, A, then release L shift then A, on an US keyboard.
//
// This is special because the characters for the A key will change in this
// process.
TEST(FlutterKeyEmbedderHandlerUnittests, ToggleModifiersDuringKeyTap) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event]];
        if (callback != nullptr) {
          callback(true, user_data);
        }
      }];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20104, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x20104, @"A", @"A", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "A");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x20104, @"A", @"A", TRUE, kKeyCodeKeyA)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "A");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", TRUE, kKeyCodeKeyA)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x20104, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];
}

TEST(FlutterKeyEmbedderHandlerUnittests, IdentifyLeftAndRightModifiers) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event]];
        if (callback != nullptr) {
          callback(true, user_data);
        }
      }];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20106, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20104, @"", @"", FALSE, kKeyCodeShiftLeft)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:^(BOOL handled) {}];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject]->data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];
}


}  // namespace flutter::testing
