// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

#include <Carbon/Carbon.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterChannelKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEmbedderKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"

namespace {
typedef void (^VoidBlock)();

// Someohow this pointer type must be defined as a single type for the compiler
// to compile the function pointer type (due to _Nullable).
typedef NSResponder* _NSResponderPtr;
typedef _Nullable _NSResponderPtr (^NextResponderProvider)();

typedef struct {
  uint64_t fallbackKeyCode;
  uint64_t logicalKey;
} LayoutRequirement;

void fillCharToLayoutRequirement(std::map<UniChar, LayoutRequirement>& result) {
  auto fromChar = [](char c) -> uint64_t {
    return static_cast<uint64_t>(c);
  };
  result['a'] = LayoutRequirement{ 0x00, fromChar('a') };
  result['s'] = LayoutRequirement{ 0x01, fromChar('s') };

  result['1'] = LayoutRequirement{ 0x12, fromChar('1') };
  result['2'] = LayoutRequirement{ 0x13, fromChar('2') };
}

UniChar ConvertKeyCodeToText(const UCKeyboardLayout* layout, int keyCode, bool shift) {
  UInt32 dead_key_state = 0;
  UniCharCount string_length = 0;
  UniChar actual_string;

  // Convert EventRecord modifiers to format UCKeyTranslate accepts. See docs
  // on UCKeyTranslate for more info.
  UInt32 modifier_state = 0;
  if (shift) {
    modifier_state = modifier_state | shiftKey;
  }
  modifier_state = (modifier_state >> 8) & 0xFF;

  OSStatus status = UCKeyTranslate(
      layout,
      static_cast<UInt16>(keyCode),
      kUCKeyActionDown,
      modifier_state,
      LMGetKbdLast(),
      kUCKeyTranslateNoDeadKeysBit,
      &dead_key_state,
      1,
      &string_length,
      &actual_string);

  if (status == noErr && string_length == 1 && !std::iscntrl(actual_string) && dead_key_state == 0) {
    return actual_string;
  }
  return 0;
}

}

@interface FlutterKeyboardManager ()

/**
 * The text input plugin set by initialization.
 */
@property(nonatomic) id<FlutterKeyboardViewDelegate> viewDelegate;

/**
 * The primary responders added by addPrimaryResponder.
 */
@property(nonatomic) NSMutableArray<id<FlutterKeyPrimaryResponder>>* primaryResponders;

@property(nonatomic) NSMutableArray<NSEvent*>* pendingTextEvents;

@property(nonatomic) BOOL processingEvent;

@property(nonatomic) NSMutableDictionary<NSNumber*, NSNumber*>* overrideLayoutMap;

/**
 * Add a primary responder, which asynchronously decides whether to handle an
 * event.
 */
- (void)addPrimaryResponder:(nonnull id<FlutterKeyPrimaryResponder>)responder;

- (void)dispatchToSecondaryResponders:(NSEvent*)event;

- (void)buildLayout;

@end

namespace {
typedef void (*VoidCallback)();

typedef struct {
  FlutterKeyboardManager* manager;
} NotificationCallbackData;

void NotificationCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
  NotificationCallbackData *data = reinterpret_cast<NotificationCallbackData*>(observer);
  if (data->manager != nil) {
    [data->manager buildLayout];
  }
}

void RegisterKeyboardLayoutChangeListener(NotificationCallbackData *data) {
  CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();

  // add an observer
  CFNotificationCenterAddObserver(center, data, NotificationCallback,
    kTISNotifySelectedKeyboardInputSourceChanged, NULL,
    CFNotificationSuspensionBehaviorDeliverImmediately
  );
}
}

@implementation FlutterKeyboardManager {
  NextResponderProvider _getNextResponder;
}

- (nonnull instancetype)initWithViewDelegate:(nonnull id<FlutterKeyboardViewDelegate>)viewDelegate {
  self = [super init];
  if (self != nil) {
    _viewDelegate = viewDelegate;

    _primaryResponders = [[NSMutableArray alloc] init];
    [self addPrimaryResponder:[[FlutterEmbedderKeyResponder alloc]
                                  initWithSendEvent:^(const FlutterKeyEvent& event,
                                                      FlutterKeyEventCallback callback,
                                                      void* userData) {
                                    [_viewDelegate sendKeyEvent:event
                                                       callback:callback
                                                       userData:userData];
                                  }]];
    [self
        addPrimaryResponder:[[FlutterChannelKeyResponder alloc]
                                initWithChannel:[FlutterBasicMessageChannel
                                                    messageChannelWithName:@"flutter/keyevent"
                                                           binaryMessenger:[_viewDelegate
                                                                               getBinaryMessenger]
                                                                     codec:[FlutterJSONMessageCodec
                                                                               sharedInstance]]]];
    _pendingTextEvents = [[NSMutableArray alloc] init];
    _overrideLayoutMap = [NSMutableDictionary<NSNumber*, NSNumber*> dictionary];
    [self buildLayout];
    for (id<FlutterKeyPrimaryResponder> responder in _primaryResponders) {
      responder.overrideLayoutMap = _overrideLayoutMap;
    }

    __weak __typeof__(self) weakSelf = self;
    NotificationCallbackData* notification = new NotificationCallbackData{
      weakSelf
    };
    RegisterKeyboardLayoutChangeListener(notification);
    // TODO: Unregister
  }
  return self;
}

- (void)addPrimaryResponder:(nonnull id<FlutterKeyPrimaryResponder>)responder {
  [_primaryResponders addObject:responder];
}

- (void)handleEvent:(nonnull NSEvent*)event {
  // Be sure to add a handling method in propagateKeyEvent when allowing more
  // event types here.
  if (event.type != NSEventTypeKeyDown && event.type != NSEventTypeKeyUp &&
      event.type != NSEventTypeFlagsChanged) {
    return;
  }

  if (_viewDelegate.isComposing) {
    [self dispatchToSecondaryResponders:event];
    return;
  }

  // Having no primary responders require extra logic, but Flutter hard-codes
  // all primary responders, so this is a situation that Flutter will never
  // encounter.
  NSAssert([_primaryResponders count] >= 0, @"At least one primary responder must be added.");

  __weak __typeof__(self) weakSelf = self;
  __block int unreplied = [_primaryResponders count];
  __block BOOL anyHandled = false;
  FlutterAsyncKeyCallback replyCallback = ^(BOOL handled) {
    unreplied -= 1;
    NSAssert(unreplied >= 0, @"More primary responders replied than possible.");
    anyHandled = anyHandled || handled;
    if (unreplied == 0 && !anyHandled) {
      [weakSelf dispatchToSecondaryResponders:event];
    }
  };

  for (id<FlutterKeyPrimaryResponder> responder in _primaryResponders) {
    [responder handleEvent:event callback:replyCallback];
  }
}

#pragma mark - Private

- (void)dispatchToSecondaryResponders:(NSEvent*)event {
  if ([_viewDelegate onTextInputKeyEvent:event]) {
    return;
  }
  NSResponder* nextResponder = _viewDelegate.nextResponder;
  if (nextResponder == nil) {
    return;
  }
  switch (event.type) {
    case NSEventTypeKeyDown:
      if ([nextResponder respondsToSelector:@selector(keyDown:)]) {
        [nextResponder keyDown:event];
      }
      break;
    case NSEventTypeKeyUp:
      if ([nextResponder respondsToSelector:@selector(keyUp:)]) {
        [nextResponder keyUp:event];
      }
      break;
    case NSEventTypeFlagsChanged:
      if ([nextResponder respondsToSelector:@selector(flagsChanged:)]) {
        [nextResponder flagsChanged:event];
      }
      break;
    default:
      NSAssert(false, @"Unexpected key event type (got %lu).", event.type);
  }
}

- (void)buildLayout {
  NSLog(@"# Building");
  [_overrideLayoutMap removeAllObjects];
  std::map<UniChar, LayoutRequirement> charToLayoutRequirement;
  fillCharToLayoutRequirement(charToLayoutRequirement);

  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  CFDataRef layout_data = static_cast<CFDataRef>((TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));
  if (!layout_data) {
    // TISGetInputSourceProperty returns null with Japanese keyboard layout.
    // Using TISCopyCurrentKeyboardLayoutInputSource to fix NULL return.
    source = TISCopyCurrentKeyboardLayoutInputSource();
    layout_data = static_cast<CFDataRef>((TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));
  }
  const UCKeyboardLayout* layout = reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layout_data));

  for (int keyCode = 0; keyCode < 128; keyCode += 1) {
    const int kCharTypes = 2;
    UniChar keyChars[kCharTypes] = {
      ConvertKeyCodeToText(layout, keyCode, false),
      ConvertKeyCodeToText(layout, keyCode, true),
    };
    NSLog(@"GetChar 0x%x -> 0x%x 0x%x", keyCode, keyChars[0], keyChars[1]);
    for (int charIdx = 0; charIdx < kCharTypes; charIdx += 1) {
      UniChar candidateChar = keyChars[charIdx];
      auto requirementIter = charToLayoutRequirement.find(candidateChar);
      if (requirementIter != charToLayoutRequirement.end()) {
        NSAssert(_overrideLayoutMap[@(keyCode)] == nil, @"Attempt to override an already overridden key code.");
        _overrideLayoutMap[@(keyCode)] = @(requirementIter->second.logicalKey);
        charToLayoutRequirement.erase(requirementIter);
        break;
      }
    }
  }

  for (auto requirementIter : charToLayoutRequirement) {
    _overrideLayoutMap[@(requirementIter.second.fallbackKeyCode)] =
        @(requirementIter.second.logicalKey);
    NSLog(@"Fallback: char %c", requirementIter.first);
  }
  NSLog(@"Override:");
  for (NSNumber* key in _overrideLayoutMap) {
      NSNumber* value = _overrideLayoutMap[key];
      NSLog(@"KeyCode: 0x%llx Logical: 0x%llx", [key unsignedLongLongValue], [value unsignedLongLongValue]);
  }
}

@end
