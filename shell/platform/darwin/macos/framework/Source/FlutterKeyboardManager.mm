// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

#include <Carbon/Carbon.h>
#include <cctype>
#include <map>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterChannelKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEmbedderKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/KeyCodeMap_Internal.h"

namespace {
typedef void (^VoidBlock)();

// Someohow this pointer type must be defined as a single type for the compiler
// to compile the function pointer type (due to _Nullable).
typedef NSResponder* _NSResponderPtr;
typedef _Nullable _NSResponderPtr (^NextResponderProvider)();

const uint64_t kDeadKeyPlane = 0x0300000000;

typedef std::pair<uint32_t, bool> LayoutClue;
static LayoutClue DetectLayoutClue(const UCKeyboardLayout* layout, uint16_t keyCode, bool shift) {
  UInt32 deadKeyState = 0;
  UniCharCount stringLength = 0;
  UniChar resultChar;

  UInt32 modifierState = ((shift ? shiftKey : 0) >> 8) & 0xFF;
  UInt32 keyboardType = LMGetKbdLast();

  bool isDeadKey = false;
  OSStatus status =
      UCKeyTranslate(layout, keyCode, kUCKeyActionDown, modifierState, keyboardType,
                     kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 1, &stringLength, &resultChar);
  // For dead keys, press the same key again to get the printable representation of the key.
  if (status == noErr && stringLength == 0 && deadKeyState != 0) {
    isDeadKey = true;
    status =
        UCKeyTranslate(layout, keyCode, kUCKeyActionDown, modifierState, keyboardType,
                       kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 1, &stringLength, &resultChar);
  }

  if (status == noErr && stringLength == 1 && !std::iscntrl(resultChar)) {
    return LayoutClue(resultChar, isDeadKey);
  }
  return LayoutClue(0, false);
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

@property(nonatomic) NSMutableDictionary<NSNumber*, NSNumber*>* layoutMap;

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

void NotificationCallback(CFNotificationCenterRef center,
                          void* observer,
                          CFStringRef name,
                          const void* object,
                          CFDictionaryRef userInfo) {
  NotificationCallbackData* data = reinterpret_cast<NotificationCallbackData*>(observer);
  if (data->manager != nil) {
    [data->manager buildLayout];
  }
}

void RegisterKeyboardLayoutChangeListener(NotificationCallbackData* data) {
  CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();

  // add an observer
  CFNotificationCenterAddObserver(center, data, NotificationCallback,
                                  kTISNotifySelectedKeyboardInputSourceChanged, NULL,
                                  CFNotificationSuspensionBehaviorDeliverImmediately);
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
    _layoutMap = [NSMutableDictionary<NSNumber*, NSNumber*> dictionary];
    [self buildLayout];
    for (id<FlutterKeyPrimaryResponder> responder in _primaryResponders) {
      responder.layoutMap = _layoutMap;
    }

    __weak __typeof__(self) weakSelf = self;
    NotificationCallbackData* notification = new NotificationCallbackData{weakSelf};
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
  [_layoutMap removeAllObjects];

  std::map<uint32_t, LayoutGoal> remainingGoals;
  for (const LayoutGoal& goal : layoutGoals) {
    remainingGoals[goal.keyChar] = goal;
  }

  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  CFDataRef layout_data =
      static_cast<CFDataRef>((TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));
  if (!layout_data) {
    CFRelease(source);
    // TISGetInputSourceProperty returns null with Japanese keyboard layout.
    // Using TISCopyCurrentKeyboardLayoutInputSource to fix NULL return.
    // https://github.com/microsoft/node-native-keymap/blob/5f0699ded00179410a14c0e1b0e089fe4df8e130/src/keyboard_mac.mm#L91
    source = TISCopyCurrentKeyboardLayoutInputSource();
    layout_data = static_cast<CFDataRef>(
        (TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));
  }
  const UCKeyboardLayout* layout =
      reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layout_data));

  // Derive key mapping for each key code based on their layout clues.
  // Max key code is 127 for ADB keyboards.
  // https://developer.apple.com/documentation/coreservices/1390584-uckeytranslate?language=objc#parameters
  const uint16_t kMaxKeyCode = 127;
  for (uint16_t keyCode = 0; keyCode <= kMaxKeyCode; keyCode += 1) {
    NSLog(@"# Keycode 0x%x", keyCode);
    const int kNumClueTypes = 2;
    LayoutClue thisKeyClues[kNumClueTypes] = {
        DetectLayoutClue(layout, keyCode, false),
        DetectLayoutClue(layout, keyCode, true),
    };
    // The logical key should be the first available clue from below:
    //
    //  - Mandatory key
    //  - Primary EASCII
    //  - Non-primary EASCII
    //  - Primary dead key
    //  - Non-primary dead key
    //  - US layout
    uint32_t easciiKey = 0;
    uint32_t deadKey = 0;
    for (int clueId = 0; clueId < kNumClueTypes; clueId += 1) {
      LayoutClue clue = thisKeyClues[clueId];
      uint32_t keyChar = clue.first;
      bool isDeadKey = clue.second;
      NSLog(@"Char 0x%x dead %d", keyChar, isDeadKey);
      auto matchingGoal = remainingGoals.find(keyChar);
      if (!isDeadKey) {
        if (matchingGoal != remainingGoals.end() && matchingGoal->second.mandatory) {
          // Found a key that produces a mandatory char. Use it.
          NSAssert(_layoutMap[@(keyCode)] == nil, @"Attempting to assign an assigned key code.");
          _layoutMap[@(keyCode)] = @(keyChar);
          remainingGoals.erase(matchingGoal);
          NSLog(@"From req");
          break;
        }
        if (easciiKey == 0 && keyChar < 256) {
          easciiKey = keyChar;
        }
      } else {
        if (deadKey == 0) {
          deadKey = keyChar;
        }
      }
    }
    // See if any produced char meets the requirement as a logical key.
    if (_layoutMap[@(keyCode)] == nil) {
      NSLog(@"Trying eascii 0x%x dead 0x%x", easciiKey, deadKey);
      if (easciiKey != 0) {
        NSLog(@"From EASCII");
        _layoutMap[@(keyCode)] = @(easciiKey);
      } else if (deadKey != 0) {
        NSLog(@"From dead");
        _layoutMap[@(keyCode)] = @(deadKey + kDeadKeyPlane);
      }
    }
  }

  // For the unfulfilled goals: mandatory goals should always overwrite
  // a map, while non-mandatory goals should only overwrite empty maps.
  for (auto uncompletedGoalIter : remainingGoals) {
    const LayoutGoal& goal = uncompletedGoalIter.second;
    NSLog(@"# Key 0x%hx char 0x%x from US req", goal.keyCode, goal.keyChar);
    if (goal.mandatory || _layoutMap[@(goal.keyCode)] == nil) {
      _layoutMap[@(goal.keyCode)] = @(goal.keyChar);
    }
  }
  CFRelease(source);
}

@end
