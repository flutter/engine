// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>
#include <Carbon/Carbon.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"
#import "flutter/testing/testing.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "flutter/shell/platform/embedder/test_utils/key_codes.h"

namespace {

using namespace flutter::testing::keycodes;

typedef BOOL (^BoolGetter)();
typedef void (^AsyncKeyCallback)(BOOL handled);
typedef void (^AsyncKeyCallbackHandler)(AsyncKeyCallback callback);
typedef void (^AsyncEmbedderCallbackHandler)(const FlutterKeyEvent* event, AsyncKeyCallback callback);

typedef const std::array<uint32_t, 256> MockLayoutData;

void clearEvents(std::vector<FlutterKeyEvent>& events) {
  for (FlutterKeyEvent& event : events) {
    if (event.character != nullptr) {
      delete[] event.character;
    }
  }
  events.clear();
}

#define VERIFY_DOWN(OUT_LOGICAL, OUT_CHAR) \
    EXPECT_EQ(events[0].type, kFlutterKeyEventTypeDown); \
    EXPECT_EQ(events[0].logical, static_cast<uint64_t>(OUT_LOGICAL)); \
    EXPECT_STREQ(events[0].character, (OUT_CHAR)); \
    clearEvents(events);

MockLayoutData kUsLayout = {
    //         +0x0     Shift    +0x1     Shift    +0x2     Shift    +0x3     Shift
    /* 0x00 */ 0x00061, 0x00041, 0x00073, 0x00053, 0x00064, 0x00044, 0x00066, 0x00046,
    /* 0x04 */ 0x00068, 0x00048, 0x00067, 0x00047, 0x0007a, 0x0005a, 0x00078, 0x00058,
    /* 0x08 */ 0x00063, 0x00043, 0x00076, 0x00056, 0x000a7, 0x000b1, 0x00062, 0x00042,
    /* 0x0c */ 0x00071, 0x00051, 0x00077, 0x00057, 0x00065, 0x00045, 0x00072, 0x00052,
    /* 0x10 */ 0x00079, 0x00059, 0x00074, 0x00054, 0x00031, 0x00021, 0x00032, 0x00040,
    /* 0x14 */ 0x00033, 0x00023, 0x00034, 0x00024, 0x00036, 0x0005e, 0x00035, 0x00025,
    /* 0x18 */ 0x0003d, 0x0002b, 0x00039, 0x00028, 0x00037, 0x00026, 0x0002d, 0x0005f,
    /* 0x1c */ 0x00038, 0x0002a, 0x00030, 0x00029, 0x0005d, 0x0007d, 0x0006f, 0x0004f,
    /* 0x20 */ 0x00075, 0x00055, 0x0005b, 0x0007b, 0x00069, 0x00049, 0x00070, 0x00050,
    /* 0x24 */ 0x00000, 0x00000, 0x0006c, 0x0004c, 0x0006a, 0x0004a, 0x00027, 0x00022,
    /* 0x28 */ 0x0006b, 0x0004b, 0x0003b, 0x0003a, 0x0005c, 0x0007c, 0x0002c, 0x0003c,
    /* 0x2c */ 0x0002f, 0x0003f, 0x0006e, 0x0004e, 0x0006d, 0x0004d, 0x0002e, 0x0003e,
    /* 0x30 */ 0x00000, 0x00000, 0x00020, 0x00020, 0x00060, 0x0007e, 0x00000, 0x00000,
    /* 0x34 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x38 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x3c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x40 */ 0x00000, 0x00000, 0x0002e, 0x0002e, 0x00000, 0x0002a, 0x0002a, 0x0002a,
    /* 0x44 */ 0x00000, 0x00000, 0x0002b, 0x0002b, 0x00000, 0x0002b, 0x00000, 0x00000,
    /* 0x48 */ 0x00000, 0x0003d, 0x00000, 0x00000, 0x00000, 0x00000, 0x0002f, 0x0002f,
    /* 0x4c */ 0x00000, 0x00000, 0x00000, 0x0002f, 0x0002d, 0x0002d, 0x00000, 0x00000,
    /* 0x50 */ 0x00000, 0x00000, 0x0003d, 0x0003d, 0x00030, 0x00030, 0x00031, 0x00031,
    /* 0x54 */ 0x00032, 0x00032, 0x00033, 0x00033, 0x00034, 0x00034, 0x00035, 0x00035,
    /* 0x58 */ 0x00036, 0x00036, 0x00037, 0x00037, 0x00000, 0x00000, 0x00038, 0x00038,
    /* 0x5c */ 0x00039, 0x00039, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x60 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x64 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x68 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x6c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x70 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x74 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x78 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x7c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

MockLayoutData kFrenchLayout = {
    //         +0x0     Shift    +0x1     Shift    +0x2     Shift    +0x3     Shift
    /* 0x00 */ 0x00071, 0x00051, 0x00073, 0x00053, 0x00064, 0x00044, 0x00066, 0x00046,
    /* 0x04 */ 0x00068, 0x00048, 0x00067, 0x00047, 0x00077, 0x00057, 0x00078, 0x00058,
    /* 0x08 */ 0x00063, 0x00043, 0x00076, 0x00056, 0x00040, 0x00023, 0x00062, 0x00042,
    /* 0x0c */ 0x00061, 0x00041, 0x0007a, 0x0005a, 0x00065, 0x00045, 0x00072, 0x00052,
    /* 0x10 */ 0x00079, 0x00059, 0x00074, 0x00054, 0x00026, 0x00031, 0x000e9, 0x00032,
    /* 0x14 */ 0x00022, 0x00033, 0x00027, 0x00034, 0x000a7, 0x00036, 0x00028, 0x00035,
    /* 0x18 */ 0x0002d, 0x0005f, 0x000e7, 0x00039, 0x000e8, 0x00037, 0x00029, 0x000b0,
    /* 0x1c */ 0x00021, 0x00038, 0x000e0, 0x00030, 0x00024, 0x0002a, 0x0006f, 0x0004f,
    /* 0x20 */ 0x00075, 0x00055, 0x1005e, 0x100a8, 0x00069, 0x00049, 0x00070, 0x00050,
    /* 0x24 */ 0x00000, 0x00000, 0x0006c, 0x0004c, 0x0006a, 0x0004a, 0x000f9, 0x00025,
    /* 0x28 */ 0x0006b, 0x0004b, 0x0006d, 0x0004d, 0x10060, 0x000a3, 0x0003b, 0x0002e,
    /* 0x2c */ 0x0003d, 0x0002b, 0x0006e, 0x0004e, 0x0002c, 0x0003f, 0x0003a, 0x0002f,
    /* 0x30 */ 0x00000, 0x00000, 0x00020, 0x00020, 0x0003c, 0x0003e, 0x00000, 0x00000,
    /* 0x34 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x38 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x3c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x40 */ 0x00000, 0x00000, 0x0002c, 0x0002e, 0x00000, 0x0002a, 0x0002a, 0x0002a,
    /* 0x44 */ 0x00000, 0x00000, 0x0002b, 0x0002b, 0x00000, 0x0002b, 0x00000, 0x00000,
    /* 0x48 */ 0x00000, 0x0003d, 0x00000, 0x00000, 0x00000, 0x00000, 0x0002f, 0x0002f,
    /* 0x4c */ 0x00000, 0x00000, 0x00000, 0x0002f, 0x0002d, 0x0002d, 0x00000, 0x00000,
    /* 0x50 */ 0x00000, 0x00000, 0x0003d, 0x0003d, 0x00030, 0x00030, 0x00031, 0x00031,
    /* 0x54 */ 0x00032, 0x00032, 0x00033, 0x00033, 0x00034, 0x00034, 0x00035, 0x00035,
    /* 0x58 */ 0x00036, 0x00036, 0x00037, 0x00037, 0x00000, 0x00000, 0x00038, 0x00038,
    /* 0x5c */ 0x00039, 0x00039, 0x00040, 0x00023, 0x0003c, 0x0003e, 0x00000, 0x00000,
    /* 0x60 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x64 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x68 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x6c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x70 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x74 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x78 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x7c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

MockLayoutData kRussianLayout = {
    //         +0x0     Shift    +0x1     Shift    +0x2     Shift    +0x3     Shift
    /* 0x00 */ 0x00444, 0x00424, 0x0044b, 0x0042b, 0x00432, 0x00412, 0x00430, 0x00410,
    /* 0x04 */ 0x00440, 0x00420, 0x0043f, 0x0041f, 0x0044f, 0x0042f, 0x00447, 0x00427,
    /* 0x08 */ 0x00441, 0x00421, 0x0043c, 0x0041c, 0x0003e, 0x0003c, 0x00438, 0x00418,
    /* 0x0c */ 0x00439, 0x00419, 0x00446, 0x00426, 0x00443, 0x00423, 0x0043a, 0x0041a,
    /* 0x10 */ 0x0043d, 0x0041d, 0x00435, 0x00415, 0x00031, 0x00021, 0x00032, 0x00022,
    /* 0x14 */ 0x00033, 0x02116, 0x00034, 0x00025, 0x00036, 0x0002c, 0x00035, 0x0003a,
    /* 0x18 */ 0x0003d, 0x0002b, 0x00039, 0x00028, 0x00037, 0x0002e, 0x0002d, 0x0005f,
    /* 0x1c */ 0x00038, 0x0003b, 0x00030, 0x00029, 0x0044a, 0x0042a, 0x00449, 0x00429,
    /* 0x20 */ 0x00433, 0x00413, 0x00445, 0x00425, 0x00448, 0x00428, 0x00437, 0x00417,
    /* 0x24 */ 0x00000, 0x00000, 0x00434, 0x00414, 0x0043e, 0x0041e, 0x0044d, 0x0042d,
    /* 0x28 */ 0x0043b, 0x0041b, 0x00436, 0x00416, 0x00451, 0x00401, 0x00431, 0x00411,
    /* 0x2c */ 0x0002f, 0x0003f, 0x00442, 0x00422, 0x0044c, 0x0042c, 0x0044e, 0x0042e,
    /* 0x30 */ 0x00000, 0x00000, 0x00020, 0x00020, 0x0005d, 0x0005b, 0x00000, 0x00000,
    /* 0x34 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x38 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x3c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x40 */ 0x00000, 0x00000, 0x0002c, 0x0002e, 0x00000, 0x0002a, 0x0002a, 0x0002a,
    /* 0x44 */ 0x00000, 0x00000, 0x0002b, 0x0002b, 0x00000, 0x0002b, 0x00000, 0x00000,
    /* 0x48 */ 0x00000, 0x0003d, 0x00000, 0x00000, 0x00000, 0x00000, 0x0002f, 0x0002f,
    /* 0x4c */ 0x00000, 0x00000, 0x00000, 0x0002f, 0x0002d, 0x0002d, 0x00000, 0x00000,
    /* 0x50 */ 0x00000, 0x00000, 0x0003d, 0x0003d, 0x00030, 0x00030, 0x00031, 0x00031,
    /* 0x54 */ 0x00032, 0x00032, 0x00033, 0x00033, 0x00034, 0x00034, 0x00035, 0x00035,
    /* 0x58 */ 0x00036, 0x00036, 0x00037, 0x00037, 0x00000, 0x00000, 0x00038, 0x00038,
    /* 0x5c */ 0x00039, 0x00039, 0x0003e, 0x0003c, 0x0005d, 0x0005b, 0x00000, 0x00000,
    /* 0x60 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x64 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x68 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x6c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x70 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x74 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x78 */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    /* 0x7c */ 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

NSEvent* keyDownEvent(unsigned short keyCode, NSString* chars = @"", NSString* charsUnmod = @"") {
  return [NSEvent keyEventWithType:NSEventTypeKeyDown
                          location:NSZeroPoint
                     modifierFlags:0x100
                         timestamp:0
                      windowNumber:0
                           context:nil
                        characters:chars
       charactersIgnoringModifiers:charsUnmod
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

}  // namespace

@interface KeyboardTester : NSObject
- (nonnull instancetype)init;
- (void)respondEmbedderCallsWith:(BOOL)response;
- (void)recordEmbedderCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage;
- (void)recordEmbedderEventsTo:(nonnull std::vector<FlutterKeyEvent>*)storage returning:(bool)handled;
- (void)respondChannelCallsWith:(BOOL)response;
- (void)recordChannelCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage;

@property(nonatomic) FlutterKeyboardManager* manager;
@property(nonatomic) NSResponder* nextResponder;
@property(nonatomic, assign) BOOL isComposing;

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
  AsyncEmbedderCallbackHandler _embedderHandler;
  AsyncKeyCallbackHandler _channelHandler;
  BOOL _textInputResponse;

  KeyboardLayoutNotifier _keyboardLayoutNotifier;
  const MockLayoutData* _currentLayout;
}

- (nonnull instancetype)init {
  self = [super init];
  if (self == nil) {
    return nil;
  }

  _nextResponder = OCMClassMock([NSResponder class]);
  [self respondChannelCallsWith:FALSE];
  [self respondEmbedderCallsWith:FALSE];
  [self respondTextInputWith:FALSE];
  _isComposing = NO;

  _currentLayout = &kUsLayout;

  id messengerMock = OCMStrictProtocolMock(@protocol(FlutterBinaryMessenger));
  OCMStub([messengerMock sendOnChannel:@"flutter/keyevent"
                               message:[OCMArg any]
                           binaryReply:[OCMArg any]])
      .andCall(self, @selector(handleChannelMessage:message:binaryReply:));

  id viewDelegateMock = OCMStrictProtocolMock(@protocol(FlutterKeyboardViewDelegate));
  OCMStub([viewDelegateMock nextResponder]).andReturn(_nextResponder);
  OCMStub([viewDelegateMock onTextInputKeyEvent:[OCMArg any]])
      .andCall(self, @selector(handleTextInputKeyEvent:));
  OCMStub([viewDelegateMock getBinaryMessenger]).andReturn(messengerMock);
  OCMStub([viewDelegateMock isComposing]).andCall(self, @selector(isComposing));
  OCMStub([viewDelegateMock sendKeyEvent:FlutterKeyEvent {} callback:nil userData:nil])
      .ignoringNonObjectArgs()
      .andCall(self, @selector(handleEmbedderEvent:callback:userData:));
  OCMStub([viewDelegateMock subscribeToKeyboardLayoutChange:[OCMArg any]])
      .andCall(self, @selector(onSetKeyboardLayoutNotifier:));
  OCMStub([viewDelegateMock lookUpLayoutForKeyCode:0 shift:false])
      .ignoringNonObjectArgs()
      .andCall(self, @selector(lookUpLayoutForKeyCode:shift:));

  _manager = [[FlutterKeyboardManager alloc] initWithViewDelegate:viewDelegateMock];
  return self;
}

- (void)respondEmbedderCallsWith:(BOOL)response {
  _embedderHandler = ^(const FlutterKeyEvent* event, AsyncKeyCallback callback) {
    callback(response);
  };
}

- (void)recordEmbedderCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage {
  _embedderHandler = ^(const FlutterKeyEvent* event, AsyncKeyCallback callback) {
    [storage addObject:callback];
  };
}

- (void)recordEmbedderEventsTo:(nonnull std::vector<FlutterKeyEvent>*)storage returning:(bool)handled {
  _embedderHandler = ^(const FlutterKeyEvent* event, AsyncKeyCallback callback) {
    FlutterKeyEvent newEvent = *event;
    if (event->character != nullptr) {
      size_t charLen = strlen(event->character);
      char* newCharacter = new char[charLen+1];
      strcpy(newCharacter, event->character);
      newEvent.character = newCharacter;
    }
    storage->push_back(newEvent);
    callback(handled);
  };
}

- (void)respondChannelCallsWith:(BOOL)response {
  _channelHandler = ^(AsyncKeyCallback callback) {
    callback(response);
  };
}

- (void)recordChannelCallsTo:(nonnull NSMutableArray<FlutterAsyncKeyCallback>*)storage {
  _channelHandler = ^(AsyncKeyCallback callback) {
    [storage addObject:callback];
  };
}

- (void)respondTextInputWith:(BOOL)response {
  _textInputResponse = response;
}

- (void)setLayout:(const MockLayoutData&)layout {
  _currentLayout = &layout;
  if (_keyboardLayoutNotifier != nil) {
    _keyboardLayoutNotifier();
  }
}

#pragma mark - Private

- (void)handleEmbedderEvent:(const FlutterKeyEvent&)event
                   callback:(nullable FlutterKeyEventCallback)callback
                   userData:(nullable void*)userData {
  if (callback != nullptr) {
    _embedderHandler(&event, ^(BOOL handled) {
      callback(handled, userData);
    });
  }
}

- (void)handleChannelMessage:(NSString*)channel
                     message:(NSData* _Nullable)message
                 binaryReply:(FlutterBinaryReply _Nullable)callback {
  _channelHandler(^(BOOL handled) {
    NSDictionary* result = @{
      @"handled" : @(handled),
    };
    NSData* encodedKeyEvent = [[FlutterJSONMessageCodec sharedInstance] encode:result];
    callback(encodedKeyEvent);
  });
}

- (BOOL)handleTextInputKeyEvent:(NSEvent*)event {
  return _textInputResponse;
}

- (void)onSetKeyboardLayoutNotifier:(nullable KeyboardLayoutNotifier)callback {
  _keyboardLayoutNotifier = callback;
}

- (LayoutClue)lookUpLayoutForKeyCode:(uint16_t)keyCode shift:(BOOL)shift {
  uint32_t cluePair = (*_currentLayout)[(keyCode * 2) + (shift ? 1 : 0)];
  const uint32_t kCharMask = 0xffff;
  const uint32_t kDeadKeyMask = 0x10000;
  // NSLog(@"%% From lookUpLayoutForKeyCode 0x%x %d -> 0x%x", keyCode, shift, cluePair);
  return LayoutClue{cluePair & kCharMask, (cluePair & kDeadKeyMask) != 0};
}

@end

@interface FlutterKeyboardManagerUnittestsObjC : NSObject
- (bool)nextResponderShouldThrowOnKeyUp;
- (bool)singlePrimaryResponder;
- (bool)doublePrimaryResponder;
- (bool)textInputPlugin;
- (bool)forwardKeyEventsToSystemWhenComposing;
- (bool)emptyNextResponder;
- (bool)correctLogicalKeyForLayouts;
@end

namespace flutter::testing {
TEST(FlutterKeyboardManagerUnittests, NextResponderShouldThrowOnKeyUp) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] nextResponderShouldThrowOnKeyUp]);
}

TEST(FlutterKeyboardManagerUnittests, SinglePrimaryResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] singlePrimaryResponder]);
}

TEST(FlutterKeyboardManagerUnittests, DoublePrimaryResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] doublePrimaryResponder]);
}

TEST(FlutterKeyboardManagerUnittests, SingleFinalResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] textInputPlugin]);
}

TEST(FlutterKeyboardManagerUnittests, handlingComposingText) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] forwardKeyEventsToSystemWhenComposing]);
}

TEST(FlutterKeyboardManagerUnittests, EmptyNextResponder) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] emptyNextResponder]);
}

TEST(FlutterKeyboardManagerUnittests, CorrectLogicalKeyForLayouts) {
  ASSERT_TRUE([[FlutterKeyboardManagerUnittestsObjC alloc] correctLogicalKeyForLayouts]);
}

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

- (bool)doublePrimaryResponder {
  KeyboardTester* tester = [[KeyboardTester alloc] init];

  // Send a down event first so we can send an up event later.
  [tester respondEmbedderCallsWith:false];
  [tester respondChannelCallsWith:false];
  [tester.manager handleEvent:keyDownEvent(0x50)];

  NSMutableArray<FlutterAsyncKeyCallback>* embedderCallbacks =
      [NSMutableArray<FlutterAsyncKeyCallback> array];
  NSMutableArray<FlutterAsyncKeyCallback>* channelCallbacks =
      [NSMutableArray<FlutterAsyncKeyCallback> array];
  [tester recordEmbedderCallsTo:embedderCallbacks];
  [tester recordChannelCallsTo:channelCallbacks];

  // Case: Both responders report TRUE.
  [tester.manager handleEvent:keyUpEvent(0x50)];
  EXPECT_EQ([embedderCallbacks count], 1u);
  EXPECT_EQ([channelCallbacks count], 1u);
  embedderCallbacks[0](TRUE);
  channelCallbacks[0](TRUE);
  EXPECT_EQ([embedderCallbacks count], 1u);
  EXPECT_EQ([channelCallbacks count], 1u);
  // [tester.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [embedderCallbacks removeAllObjects];
  [channelCallbacks removeAllObjects];

  // Case: One responder reports TRUE.
  [tester respondEmbedderCallsWith:false];
  [tester respondChannelCallsWith:false];
  [tester.manager handleEvent:keyDownEvent(0x50)];

  [tester recordEmbedderCallsTo:embedderCallbacks];
  [tester recordChannelCallsTo:channelCallbacks];
  [tester.manager handleEvent:keyUpEvent(0x50)];
  EXPECT_EQ([embedderCallbacks count], 1u);
  EXPECT_EQ([channelCallbacks count], 1u);
  embedderCallbacks[0](FALSE);
  channelCallbacks[0](TRUE);
  EXPECT_EQ([embedderCallbacks count], 1u);
  EXPECT_EQ([channelCallbacks count], 1u);
  // [tester.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [embedderCallbacks removeAllObjects];
  [channelCallbacks removeAllObjects];

  // Case: Both responders report FALSE.
  [tester.manager handleEvent:keyDownEvent(0x53)];
  EXPECT_EQ([embedderCallbacks count], 1u);
  EXPECT_EQ([channelCallbacks count], 1u);
  embedderCallbacks[0](FALSE);
  channelCallbacks[0](FALSE);
  EXPECT_EQ([embedderCallbacks count], 1u);
  EXPECT_EQ([channelCallbacks count], 1u);
  OCMVerify([tester.nextResponder keyDown:checkKeyDownEvent(0x53)]);
  [embedderCallbacks removeAllObjects];
  [channelCallbacks removeAllObjects];

  return true;
}

- (bool)textInputPlugin {
  KeyboardTester* tester = [[KeyboardTester alloc] init];

  // Send a down event first so we can send an up event later.
  [tester respondEmbedderCallsWith:false];
  [tester respondChannelCallsWith:false];
  [tester.manager handleEvent:keyDownEvent(0x50)];

  NSMutableArray<FlutterAsyncKeyCallback>* callbacks =
      [NSMutableArray<FlutterAsyncKeyCallback> array];
  [tester recordEmbedderCallsTo:callbacks];

  // Case: Primary responder responds TRUE. The event shouldn't be handled by
  // the secondary responder.
  [tester respondTextInputWith:FALSE];
  [tester.manager handleEvent:keyUpEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](TRUE);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [callbacks removeAllObjects];

  // Send a down event first so we can send an up event later.
  [tester respondEmbedderCallsWith:false];
  [tester.manager handleEvent:keyDownEvent(0x50)];

  // Case: Primary responder responds FALSE. The secondary responder returns
  // TRUE.
  [tester recordEmbedderCallsTo:callbacks];
  [tester respondTextInputWith:TRUE];
  [tester.manager handleEvent:keyUpEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](FALSE);
  // [owner.nextResponder keyUp:] should not be called, otherwise an error will be thrown.
  [callbacks removeAllObjects];

  // Case: Primary responder responds FALSE. The secondary responder returns FALSE.
  [tester respondTextInputWith:FALSE];
  [tester.manager handleEvent:keyDownEvent(0x50)];
  EXPECT_EQ([callbacks count], 1u);
  callbacks[0](FALSE);
  OCMVerify([tester.nextResponder keyDown:checkKeyDownEvent(0x50)]);
  [callbacks removeAllObjects];

  return true;
}

- (bool)forwardKeyEventsToSystemWhenComposing {
  KeyboardTester* tester = OCMPartialMock([[KeyboardTester alloc] init]);

  NSMutableArray<FlutterAsyncKeyCallback>* channelCallbacks =
      [NSMutableArray<FlutterAsyncKeyCallback> array];
  NSMutableArray<FlutterAsyncKeyCallback>* embedderCallbacks =
      [NSMutableArray<FlutterAsyncKeyCallback> array];
  [tester recordEmbedderCallsTo:embedderCallbacks];
  [tester recordChannelCallsTo:channelCallbacks];
  // The event shouldn't propagate further even if TextInputPlugin does not
  // claim the event.
  [tester respondTextInputWith:NO];

  tester.isComposing = YES;
  // Send a down event with composing == YES.
  [tester.manager handleEvent:keyUpEvent(0x50)];

  // Nobody gets the event except for the text input plugin.
  EXPECT_EQ([channelCallbacks count], 0u);
  EXPECT_EQ([embedderCallbacks count], 0u);
  OCMVerify(times(1), [tester handleTextInputKeyEvent:checkKeyDownEvent(0x50)]);

  return true;
}

- (bool)emptyNextResponder {
  KeyboardTester* tester = [[KeyboardTester alloc] init];
  tester.nextResponder = nil;

  [tester respondEmbedderCallsWith:false];
  [tester respondChannelCallsWith:false];
  [tester respondTextInputWith:false];
  [tester.manager handleEvent:keyDownEvent(0x50)];

  // Passes if no error is thrown.
  return true;
}

- (bool)correctLogicalKeyForLayouts {
  KeyboardTester* tester = [[KeyboardTester alloc] init];
  tester.nextResponder = nil;

  std::vector<FlutterKeyEvent> events;
  [tester recordEmbedderEventsTo:&events returning:true];
  [tester respondChannelCallsWith:false];
  [tester respondTextInputWith:false];

  auto sendTap = [&](uint16_t keyCode, NSString* chars, NSString* charsUnmod) {
    [tester.manager handleEvent:keyDownEvent(keyCode, chars, charsUnmod)];
    [tester.manager handleEvent:keyUpEvent(keyCode)];
  };

  /* US keyboard layout */

  sendTap(kVK_ANSI_A, @"a", @"a"); // KeyA
  VERIFY_DOWN(kLogicalKeyA, "a");

  sendTap(kVK_ANSI_A, @"A", @"A"); // Shift-KeyA
  VERIFY_DOWN(kLogicalKeyA, "A");

  sendTap(kVK_ANSI_A, @"å", @"a"); // Option-KeyA
  VERIFY_DOWN(kLogicalKeyA, "å");

  sendTap(kVK_ANSI_T, @"t", @"t"); // KeyT
  VERIFY_DOWN(kLogicalKeyT, "t");

  sendTap(kVK_ANSI_1, @"1", @"1"); // Digit1
  VERIFY_DOWN(kLogicalDigit1, "1");

  sendTap(kVK_ANSI_1, @"!", @"!"); // Shift-Digit1
  VERIFY_DOWN(kLogicalDigit1, "!");

  sendTap(kVK_ANSI_Minus, @"-", @"-"); // Minus
  VERIFY_DOWN('-', "-");

  sendTap(kVK_ANSI_Minus, @"=", @"="); // Shift-Minus
  VERIFY_DOWN('=', "=");

  /* French keyboard layout */
  [tester setLayout:kFrenchLayout];

  sendTap(kVK_ANSI_A, @"q", @"q"); // KeyA
  VERIFY_DOWN(kLogicalKeyQ, "q");

  sendTap(kVK_ANSI_A, @"Q", @"Q"); // Shift-KeyA
  VERIFY_DOWN(kLogicalKeyQ, "Q");

  sendTap(kVK_ANSI_Semicolon, @"m", @"m"); // ; but prints M
  VERIFY_DOWN(kLogicalKeyM, "m");

  sendTap(kVK_ANSI_M, @",", @","); // M but prints ,
  VERIFY_DOWN(',', ",");

  sendTap(kVK_ANSI_1, @"&", @"&"); // Digit1
  VERIFY_DOWN(kLogicalDigit1, "&");

  sendTap(kVK_ANSI_1, @"1", @"1"); // Shift-Digit1
  VERIFY_DOWN(kLogicalDigit1, "1");

  sendTap(kVK_ANSI_Minus, @")", @")"); // Minus
  VERIFY_DOWN(')', ")");

  sendTap(kVK_ANSI_Minus, @"°", @"°"); // Shift-Minus
  VERIFY_DOWN(L'°', "°");


  /* Russian keyboard layout */
  [tester setLayout:kRussianLayout];

  sendTap(kVK_ANSI_A, @"ф", @"ф"); // KeyA
  VERIFY_DOWN(kLogicalKeyA, "ф");

  sendTap(kVK_ANSI_1, @"1", @"1"); // Digit1
  VERIFY_DOWN(kLogicalDigit1, "1");

  sendTap(kVK_ANSI_LeftBracket, @"х", @"х");
  VERIFY_DOWN(kLogicalBracketLeft, "х");

  // Passes if no error is thrown.
  return true;
}

@end
