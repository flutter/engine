// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyEmbedderHandler.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

// A wrap to convert FlutterKeyEvent to a ObjC class.
@interface TestKeyEvent : NSObject
@property(nonatomic) FlutterKeyEvent* data;
@property(nonatomic) FlutterKeyEventCallback callback;
@property(nonatomic) _VoidPtr userData;
- (nonnull instancetype)initWithEvent:(const FlutterKeyEvent*)event
                             callback:(nullable FlutterKeyEventCallback)callback
                             userData:(nullable _VoidPtr)userData;
- (BOOL)hasCallback;
- (void)respond:(BOOL)handled;
@end

@implementation TestKeyEvent
- (instancetype)initWithEvent:(const FlutterKeyEvent*)event
                     callback:(nullable FlutterKeyEventCallback)callback
                     userData:(nullable _VoidPtr)userData {
  self = [super init];
  _data = new FlutterKeyEvent(*event);
  if (event->character != nullptr) {
    size_t len = strlen(event->character);
    char* character = new char[len + 1];
    strcpy(character, event->character);
    _data->character = character;
  }
  _callback = callback;
  _userData = userData;
  return self;
}

- (BOOL)hasCallback {
  return _callback != nil;
}

- (void)respond:(BOOL)handled {
  NSAssert(_callback != nil,
           @"Only call `respond` after checking `hasCallback`.");  // Caller's responsibility
  _callback(handled, _userData);
}

- (void)dealloc {
  if (_data->character != nullptr)
    delete[] _data->character;
  delete _data;
}
@end

namespace flutter::testing {

namespace {
constexpr uint64_t kKeyCodeKeyA = 0;
constexpr uint64_t kKeyCodeShiftLeft = 0x38;
constexpr uint64_t kKeyCodeShiftRight = 0x3c;
constexpr uint64_t kKeyCodeCapsLock = 0x39;
constexpr uint64_t kKeyCodeNumpad1 = 0x53;
constexpr uint64_t kKeyCodeF1 = 0x7a;

constexpr uint64_t kPhysicalKeyA = 0x00070004;
// constexpr uint64_t kPhysicalControlLeft = 0x000700e0;
// constexpr uint64_t kPhysicalControlRight = 0x000700e4;
constexpr uint64_t kPhysicalShiftLeft = 0x000700e1;
constexpr uint64_t kPhysicalShiftRight = 0x000700e5;
constexpr uint64_t kPhysicalCapsLock = 0x00070039;
constexpr uint64_t kPhysicalNumpad1 = 0x00070059;
constexpr uint64_t kPhysicalF1 = 0x0007003a;
// constexpr uint64_t kPhysicalKeyNumLock = 0x00070053;

constexpr uint64_t kLogicalKeyA = 0x00000061;
// constexpr uint64_t kLogicalControlLeft = 0x00300000105;
// constexpr uint64_t kLogicalControlRight = 0x00400000105;
constexpr uint64_t kLogicalShiftLeft = 0x0030000010d;
constexpr uint64_t kLogicalShiftRight = 0x0040000010d;
constexpr uint64_t kLogicalCapsLock = 0x00000000104;
constexpr uint64_t kLogicalNumpad1 = 0x00200000031;
constexpr uint64_t kLogicalF1 = 0x00000000801;
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
  __block BOOL last_handled = TRUE;
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", FALSE, 0)
              callback:^(BOOL handled) {
                last_handled = handled;
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", TRUE, kKeyCodeKeyA)
              callback:^(BOOL handled) {
                last_handled = handled;
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = TRUE;
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x100, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled) {
                last_handled = handled;
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_EQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, TRUE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:FALSE];  // Check if responding FALSE works
  EXPECT_EQ(last_handled, FALSE);

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
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20104, @"", @"", FALSE, kKeyCodeShiftRight)
         callback:^(BOOL handled){
         }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x20104, @"A", @"A", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "A");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x20104, @"A", @"A", TRUE, kKeyCodeKeyA)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "A");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", TRUE, kKeyCodeKeyA)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeRepeat);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x100, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];
}

// Special modifier flags.
//
// Some keys in modifierFlags are not to indicate modifier state, but to mark
// the key area that the key belongs to, such as numpad keys or function keys.
// Ensure these flags do not obstruct other keys.
TEST(FlutterKeyEmbedderHandlerUnittests, SpecialModiferFlags) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  // Keydown:    Numpad1, F1, KeyA, ShiftLeft
  // Then KeyUp: Numpad1, F1, KeyA, ShiftLeft

  // Numpad 1
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x200100, @"1", @"1", FALSE, kKeyCodeNumpad1)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalNumpad1);
  EXPECT_EQ(event->logical, kLogicalNumpad1);
  EXPECT_STREQ(event->character, "1");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // F1
  [handler
      handleEvent:keyEvent(NSEventTypeKeyDown, 0x800100, @"\uf704", @"\uf704", FALSE, kKeyCodeF1)
         callback:^(BOOL handled){
         }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalF1);
  EXPECT_EQ(event->logical, kLogicalF1);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // KeyA
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x100, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "a");
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // ShiftLeft
  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:^(BOOL handled){
         }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // Numpad 1
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x220102, @"1", @"1", FALSE, kKeyCodeNumpad1)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalNumpad1);
  EXPECT_EQ(event->logical, kLogicalNumpad1);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // F1
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x820102, @"\uF704", @"\uF704", FALSE, kKeyCodeF1)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalF1);
  EXPECT_EQ(event->logical, kLogicalF1);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // KeyA
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x20102, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  // ShiftLeft
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftLeft)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
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
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:^(BOOL handled){
         }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20106, @"", @"", FALSE, kKeyCodeShiftRight)
         callback:^(BOOL handled){
         }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20104, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:^(BOOL handled){
         }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];

  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:^(BOOL handled){
              }];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  [events removeAllObjects];
}

// Process various cases where pair modifier key events are missed, and the
// handler has to "guess" how to synchronize states.
//
// In the following comments, parentheses indicate missed events, while
// asterisks indicate synthesized events.
TEST(FlutterKeyEmbedderHandlerUnittests, SynthesizeMissedModifierEvents) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block BOOL last_handled = TRUE;
  id keyEventCallback = ^(BOOL handled) {
    last_handled = handled;
  };
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  // Case 1:
  // In:  L down, (L up), L down, L up
  // Out: L down,                 L up
  last_handled = FALSE;
  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:keyEventCallback];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = FALSE;
  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:keyEventCallback];

  EXPECT_EQ([events count], 0u);
  EXPECT_EQ(last_handled, TRUE);

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftLeft)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  // Case 2:
  // In:  (L down), L up
  // Out:

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftLeft)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 0u);
  EXPECT_EQ(last_handled, TRUE);

  // Case 3:
  // In:  L down, (L up), (R down), R up
  // Out: L down,                   *L up

  last_handled = FALSE;
  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:keyEventCallback];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);

  // The primary event is automatically replied with TRUE, unrelated to the received event.
  EXPECT_EQ(last_handled, TRUE);
  EXPECT_FALSE([[events lastObject] hasCallback]);

  [events removeAllObjects];

  // Case 4:
  // In:  L down, R down, (L up), R up
  // Out: L down, R down          *L up & R up

  last_handled = FALSE;
  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20102, @"", @"", FALSE, kKeyCodeShiftLeft)
         callback:keyEventCallback];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = FALSE;
  [handler
      handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x20106, @"", @"", FALSE, kKeyCodeShiftRight)
         callback:keyEventCallback];

  EXPECT_EQ([events count], 1u);
  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeShiftRight)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 2u);
  event = [events firstObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);

  EXPECT_FALSE([[events firstObject] hasCallback]);

  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftRight);
  EXPECT_EQ(event->logical, kLogicalShiftRight);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);

  EXPECT_EQ(last_handled, FALSE);
  EXPECT_TRUE([[events lastObject] hasCallback]);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];
}

TEST(FlutterKeyEmbedderHandlerUnittests, SynthesizeMissedModifierEventsInNormalEvents) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block BOOL last_handled = TRUE;
  id keyEventCallback = ^(BOOL handled) {
    last_handled = handled;
  };
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  // In:  (LShift down), A down,           (LShift up), A up
  // Out:               *LS down & A down,              *LS up & A up

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x20102, @"A", @"A", FALSE, kKeyCodeKeyA)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 2u);
  event = [events firstObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);
  EXPECT_FALSE([[events firstObject] hasCallback]);

  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "A");
  EXPECT_EQ(event->synthesized, false);
  EXPECT_TRUE([[events lastObject] hasCallback]);

  EXPECT_EQ(last_handled, FALSE);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyUp, 0x100, @"a", @"a", FALSE, kKeyCodeKeyA)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 2u);
  event = [events firstObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalShiftLeft);
  EXPECT_EQ(event->logical, kLogicalShiftLeft);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);
  EXPECT_FALSE([[events firstObject] hasCallback]);

  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);
  EXPECT_TRUE([[events lastObject] hasCallback]);

  EXPECT_EQ(last_handled, FALSE);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];
}

TEST(FlutterKeyEmbedderHandlerUnittests, ConvertCapsLockEvents) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block BOOL last_handled = TRUE;
  id keyEventCallback = ^(BOOL handled) {
    last_handled = handled;
  };
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  // In:  CapsLock down
  // Out: CapsLock down & *CapsLock Up
  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x10100, @"", @"", FALSE, kKeyCodeCapsLock)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 2u);

  event = [events firstObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalCapsLock);
  EXPECT_EQ(event->logical, kLogicalCapsLock);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);
  EXPECT_TRUE([[events firstObject] hasCallback]);

  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalCapsLock);
  EXPECT_EQ(event->logical, kLogicalCapsLock);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);
  EXPECT_FALSE([[events lastObject] hasCallback]);

  EXPECT_EQ(last_handled, FALSE);
  [[events firstObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];

  // In:  CapsLock up
  // Out: CapsLock down & *CapsLock Up
  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeCapsLock)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 2u);

  event = [events firstObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalCapsLock);
  EXPECT_EQ(event->logical, kLogicalCapsLock);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, false);
  EXPECT_TRUE([[events firstObject] hasCallback]);

  event = [events lastObject].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalCapsLock);
  EXPECT_EQ(event->logical, kLogicalCapsLock);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);
  EXPECT_FALSE([[events lastObject] hasCallback]);

  EXPECT_EQ(last_handled, FALSE);
  [[events firstObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];
}

// Press the CapsLock key when CapsLock state is desynchronized
TEST(FlutterKeyEmbedderHandlerUnittests, SynchronizeCapsLockStateOnCapsLock) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block BOOL last_handled = TRUE;
  id keyEventCallback = ^(BOOL handled) {
    last_handled = handled;
  };

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  // In:  CapsLock down
  // Out:
  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeFlagsChanged, 0x100, @"", @"", FALSE, kKeyCodeCapsLock)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 0u);
  EXPECT_EQ(last_handled, TRUE);
}

// Press the CapsLock key when CapsLock state is desynchronized
TEST(FlutterKeyEmbedderHandlerUnittests, SynchronizeCapsLockStateOnNormalKey) {
  __block NSMutableArray<TestKeyEvent*>* events = [[NSMutableArray<TestKeyEvent*> alloc] init];
  __block BOOL last_handled = TRUE;
  id keyEventCallback = ^(BOOL handled) {
    last_handled = handled;
  };
  FlutterKeyEvent* event;

  FlutterKeyEmbedderHandler* handler = [[FlutterKeyEmbedderHandler alloc]
      initWithSendEvent:^(const FlutterKeyEvent& event, _Nullable FlutterKeyEventCallback callback,
                          _Nullable _VoidPtr user_data) {
        [events addObject:[[TestKeyEvent alloc] initWithEvent:&event
                                                     callback:callback
                                                     userData:user_data]];
      }];

  last_handled = FALSE;
  [handler handleEvent:keyEvent(NSEventTypeKeyDown, 0x10100, @"A", @"a", FALSE, kKeyCodeKeyA)
              callback:keyEventCallback];

  EXPECT_EQ([events count], 3u);

  event = events[0].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalCapsLock);
  EXPECT_EQ(event->logical, kLogicalCapsLock);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);
  EXPECT_FALSE([events[0] hasCallback]);

  event = events[1].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeUp);
  EXPECT_EQ(event->physical, kPhysicalCapsLock);
  EXPECT_EQ(event->logical, kLogicalCapsLock);
  EXPECT_STREQ(event->character, nullptr);
  EXPECT_EQ(event->synthesized, true);
  EXPECT_FALSE([events[1] hasCallback]);

  event = events[2].data;
  EXPECT_EQ(event->type, kFlutterKeyEventTypeDown);
  EXPECT_EQ(event->physical, kPhysicalKeyA);
  EXPECT_EQ(event->logical, kLogicalKeyA);
  EXPECT_STREQ(event->character, "A");
  EXPECT_EQ(event->synthesized, false);
  EXPECT_TRUE([events[2] hasCallback]);

  EXPECT_EQ(last_handled, FALSE);
  [[events lastObject] respond:TRUE];
  EXPECT_EQ(last_handled, TRUE);

  [events removeAllObjects];
}

}  // namespace flutter::testing
