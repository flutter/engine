// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

#include <cctype>
#include <map>
#include <Carbon/Carbon.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterChannelKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEmbedderKeyResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/KeyCodeMap_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyPrimaryResponder.h"

namespace {
typedef void (^VoidBlock)();

// Someohow this pointer type must be defined as a single type for the compiler
// to compile the function pointer type (due to _Nullable).
typedef NSResponder* _NSResponderPtr;
typedef _Nullable _NSResponderPtr (^NextResponderProvider)();

const uint64_t kDeadKeyPlane = 0x0300000000;

typedef std::pair<uint32_t, bool> KeyChar;
static KeyChar DetectKeyChar(const UCKeyboardLayout* layout, uint16_t keyCode, bool shift) {
  UInt32 deadKeyState = 0;
  UniCharCount stringLength = 0;
  UniChar resultChar;

  UInt32 modifierState = ((shift ? shiftKey : 0) >> 8) & 0xFF;
  UInt32 keyboardType = LMGetKbdLast();

  bool isDeadKey = false;
  OSStatus status = UCKeyTranslate(layout, keyCode, kUCKeyActionDown,
                                   modifierState, keyboardType, kUCKeyTranslateNoDeadKeysBit,
                                   &deadKeyState, 1, &stringLength, &resultChar);
  // For dead keys, press the same key again to get the printable representation of the key.
  if (status == noErr && stringLength == 0 && deadKeyState != 0) {
    isDeadKey = true;
    status = UCKeyTranslate(layout, keyCode, kUCKeyActionDown, modifierState,
    keyboardType, kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 1, &stringLength,
    &resultChar);
  }

  if (status == noErr && stringLength == 1 && !std::iscntrl(resultChar) && deadKeyState == 0) {
    return KeyChar(resultChar, isDeadKey);
  }
  return KeyChar(0, false);
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
    _overrideLayoutMap = [NSMutableDictionary<NSNumber*, NSNumber*> dictionary];
    [self buildLayout];
    for (id<FlutterKeyPrimaryResponder> responder in _primaryResponders) {
      responder.overrideLayoutMap = _overrideLayoutMap;
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
  [_overrideLayoutMap removeAllObjects];
  std::map<uint32_t, LayoutPreset> remainingPresets;
  for (const LayoutPreset& preset : layoutPresets) {
    remainingPresets[preset.keyChar] = preset;
  }

  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  CFDataRef layout_data =
      static_cast<CFDataRef>((TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));
  if (!layout_data) {
    // TISGetInputSourceProperty returns null with Japanese keyboard layout.
    // Using TISCopyCurrentKeyboardLayoutInputSource to fix NULL return.
    source = TISCopyCurrentKeyboardLayoutInputSource();
    layout_data = static_cast<CFDataRef>(
        (TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)));
  }
  const UCKeyboardLayout* layout =
      reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layout_data));

  // Max key code is 127 for ADB keyboards.
  // https://developer.apple.com/documentation/coreservices/1390584-uckeytranslate?language=objc#parameters
  const uint16_t kMaxKeyCode = 127;
  for (uint16_t keyCode = 0; keyCode <= kMaxKeyCode; keyCode += 1) {
    NSLog(@"# Keycode 0x%x", keyCode);
    const int kCharTypes = 2;
    KeyChar keyChars[kCharTypes] = {
        DetectKeyChar(layout, keyCode, false),
        DetectKeyChar(layout, keyCode, true),
    };
    // Determine what key to use in the following order:
    // - Mandatory key
    // - Primary EASCII
    // - Non-primary EASCII
    // - Primary Deadkey
    // - Non-primary Deadkey
    // - US layout
    uint64_t easciiKey = 0;
    uint64_t deadKey = 0;
    for (int charIdx = 0; charIdx < kCharTypes; charIdx += 1) {
      KeyChar keyCharPair = keyChars[charIdx];
      uint32_t keyChar = keyCharPair.first;
      bool isDeadKey = keyCharPair.second;
      auto presetIter = remainingPresets.find(keyChar);
      if (presetIter != remainingPresets.end() && presetIter->second.mandatory) {
        // Found a key that produces a mandatory char. Use it.
        NSAssert(_overrideLayoutMap[@(keyCode)] == nil,
                 @"Attempting to assign an assigned key code.");
        _overrideLayoutMap[@(keyCode)] = @(keyChar);
        remainingPresets.erase(presetIter);
        NSLog(@"From req");
        break;
      }
      if (easciiKey == 0 && keyChar < 256) {
        easciiKey = keyChar;
      }
      if (deadKey == 0 && isDeadKey) {
        deadKey = keyChar;
      }
    }
    // See if any produced char meets the requirement as a logical key.
    if (_overrideLayoutMap[@(keyCode)] == nil) {
      if (easciiKey != 0) {
        NSLog(@"From EASCII");
        _overrideLayoutMap[@(keyCode)] = @(easciiKey);
      } else if (deadKey != 0) {
        NSLog(@"From dead");
        _overrideLayoutMap[@(keyCode)] = @(deadKey + kDeadKeyPlane);
      }
    }
  }

  // For the unfulfilled presets: mandatory presets should always overwrite
  // a map, while non-mandatory presets should only overwrite empty maps.
  for (auto presetIter : remainingPresets) {
    const LayoutPreset& preset = presetIter.second;
    NSLog(@"# Key 0x%hx char 0x%x from US req", preset.keyCode, preset.keyChar);
    if (preset.mandatory || _overrideLayoutMap[@(preset.keyCode)] == nil) {
      _overrideLayoutMap[@(preset.keyCode)] = @(preset.keyChar);
    }
  }
}

@end
