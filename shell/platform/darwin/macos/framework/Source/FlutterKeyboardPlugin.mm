// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/embedder/embedder.h"
#import "FlutterKeyboardPlugin.h"
#import "KeyCodeMap_internal.h"

// The number of bytes that should be able to fully store `event.characters`.
//
// This is an arbitrary number that is considered sufficient. The `characters`
// takes such a small and capped amount of memory that malloc isn't worthwhile.
//
// Since `GetLogicalKeyForEvent` asserts `event.charactersIgnoringModifiers` to
// be less than 2 int16's, i.e. 4 bytes, the `event.characters` is asserted
// to be double the amount, i.e. 8 bytes.
static const size_t kMaxCharactersSize = 8;

static const uint8_t kEmptyCharacterData[] = {};

static const uint64_t kCapsLockPhysicalKey = 0x00070039;

// Map the physical key code of a key to that of its sibling key.
//
// A sibling key is the other key of a pair of keys that share the same modifier
// flag, such as left and right shift keys.
static const NSDictionary* kSiblingPhysicalKeys = @{
  @0x000700e1 : @0x000700e5,  // shift
  @0x000700e5 : @0x000700e1,  // shift
  @0x000700e0 : @0x000700e4,  // control
  @0x000700e4 : @0x000700e0,  // control
  @0x000700e2 : @0x000700e6,  // alt
  @0x000700e6 : @0x000700e2,  // alt
  @0x000700e3 : @0x000700e7,  // meta
  @0x000700e7 : @0x000700e3,  // meta
};

static const NSDictionary* kModiferFlags = @{
  @0x000700e1 : @(NSEventModifierFlagShift),    // shiftLeft
  @0x000700e5 : @(NSEventModifierFlagShift),    // shiftRight
  @0x000700e0 : @(NSEventModifierFlagControl),  // controlLeft
  @0x000700e4 : @(NSEventModifierFlagControl),  // controlRight
  @0x000700e2 : @(NSEventModifierFlagOption),   // altLeft
  @0x000700e6 : @(NSEventModifierFlagOption),   // altRight
  @0x000700e3 : @(NSEventModifierFlagCommand),  // metaLeft
  @0x000700e7 : @(NSEventModifierFlagCommand),  // metaRight
  // Don't include CapsLock here, for it is handled specially.
  // Other modifier keys do not seem to be needed.
};

static bool IsControlCharacter(NSUInteger length, NSString *label) {
  if (length > 1) {
    return false;
  }
  unichar codeUnit = [label characterAtIndex:0];
  return (codeUnit <= 0x1f && codeUnit >= 0x00) || (codeUnit >= 0x7f && codeUnit <= 0x9f);
}

static bool IsUnprintableKey(NSUInteger length, NSString *label) {
  if (length > 1) {
    return false;
  }
  unichar codeUnit = [label characterAtIndex:0];
  return codeUnit >= 0xF700 && codeUnit <= 0xF8FF;
}

static uint64_t KeyOfPlane(uint64_t baseKey, uint64_t plane) {
  return plane | (baseKey & kValueMask);
}

static uint64_t GetSiblingKeyForKey(uint64_t physicalKey) {
  NSNumber* siblingKey = [kSiblingPhysicalKeys objectForKey:@(physicalKey)];
  if (siblingKey == nil)
    return 0;
  return siblingKey.unsignedLongLongValue;
}

static uint64_t GetModifierFlagForKey(uint64_t physicalKey) {
  NSNumber* modifierFlag = [kModiferFlags objectForKey:@(physicalKey)];
  if (modifierFlag == nil)
    return 0;
  return modifierFlag.unsignedLongLongValue;
}

static uint64_t GetPhysicalKeyForEvent(NSEvent* event) {
  NSNumber* physicalKeyKey = [keyCodeToPhysicalKey objectForKey:@(event.keyCode)];
  if (physicalKeyKey == nil)
    return 0;
  return physicalKeyKey.unsignedLongLongValue;
}

static uint64_t GetLogicalKeyForModifier(uint64_t physicalKey) {
  return KeyOfPlane(physicalKey, kHidPlane);
}

// Get the logical key of a KeyUp or KeyDown event.
//
// For FlagsChanged event, use GetLogicalKeyForModifier.
static uint64_t GetLogicalKeyForEvent(NSEvent* event, uint64_t physicalKey) {
  // Look to see if the keyCode is a printable number pad key, so that a
  // difference between regular keys (e.g. "=") and the number pad version (e.g.
  // the "=" on the number pad) can be determined.
  NSNumber* numPadKey = [keyCodeToNumpad objectForKey:@(event.keyCode)];
  if (numPadKey != nil)
    return numPadKey.unsignedLongLongValue;

  NSString* keyLabel = event.charactersIgnoringModifiers;
  NSUInteger keyLabelLength = [keyLabel length];
  // If this key is printable, generate the logical key from its Unicode
  // value. Control keys such as ESC, CRTL, and SHIFT are not printable. HOME,
  // DEL, arrow keys, and function keys are considered modifier function keys,
  // which generate invalid Unicode scalar values.
  if (keyLabelLength != 0 &&
      !IsControlCharacter(keyLabelLength, keyLabel) &&
      !IsUnprintableKey(keyLabelLength, keyLabel)) {
    // Given that charactersIgnoringModifiers can contain a string of arbitrary
    // length, limit to a maximum of two Unicode scalar values. It is unlikely
    // that a keyboard would produce a code point bigger than 32 bits, but it is
    // still worth defending against this case.
    NSCAssert((keyLabelLength < 2),
        @"Unexpected long key label: |%@|. Please report this to Flutter.", keyLabel);

    uint64_t codeUnit = (uint64_t)[keyLabel characterAtIndex:0];
    if (keyLabelLength == 2) {
      uint64_t secondCode = (uint64_t)[keyLabel characterAtIndex:1];
      codeUnit = (codeUnit << 16) | secondCode;
    }
    return KeyOfPlane(codeUnit, kUnicodePlane);
  }

  // Control keys like "backspace" and movement keys like arrow keys don't have
  // a printable representation, but are present on the physical keyboard. Since
  // there is no logical keycode map for macOS (macOS uses the keycode to
  // reference physical keys), a LogicalKeyboardKey is created with the physical
  // key's HID usage and debugName. This avoids duplicating the physical key
  // map.
  if (physicalKey != 0) {
    return KeyOfPlane(physicalKey, kHidPlane);
  }

  // This is a non-printable key that is unrecognized, so a new code is minted
  // with the autogenerated bit set.
  return KeyOfPlane(event.keyCode, kMacosPlane | kAutogeneratedMask);
}

static double GetFlutterTimestampFrom(NSEvent* event) {
  // Timestamp in microseconds. The event.timestamp is in seconds with sub-ms precision.
  return event.timestamp * 1000000.0;
}

@interface FlutterKeyboardPlugin ()

/**
 * The FlutterViewController to manage input for.
 */
@property(nonatomic, weak) FlutterViewController* flutterViewController;

/**
 * A map of presessd keys.
 *
 * The keys of the dictionary are physical keys, while the values are the logical keys
 * on pressing.
 */
@property(nonatomic) NSMutableDictionary* pressedKeys;

/**
 * A bitmask indicating whether each lock is on.
 */
@property(nonatomic) NSUInteger lockFlags;

@end

@implementation FlutterKeyboardPlugin

- (instancetype)initWithViewController:(FlutterViewController*)viewController {
  self = [super init];
  if (self != nil) {
    _flutterViewController = viewController;
    _pressedKeys = [NSMutableDictionary dictionary];
  }
  return self;
}

- (void)updateKey:(uint64_t)physicalKey asPressed:(uint64_t)logicalKey {
  if (logicalKey == 0) {
    [_pressedKeys removeObjectForKey:@(physicalKey)];
  } else {
    _pressedKeys[@(physicalKey)] = @(logicalKey);
  }
}

- (void)updateLockFlag:(uint64_t)lockBit asOn:(BOOL)isOn {
  if (isOn) {
    _lockFlags |= lockBit;
  } else {
    _lockFlags &= ~lockBit;
  }
}

// Send a simple event that contains 1 logical event that has the same kind as
// the physical event and no characters.
//
// The `lockFlags` must be updated before this method if necessary.
- (void)sendSimpleEventWithKind:(FlutterKeyEventKind)kind
                    physicalKey:(uint64_t)physicalKey
                     logicalKey:(uint64_t)logicalKey
                      timestamp:(double)timestamp {
  FlutterLogicalKeyEvent logical_event = {
    .struct_size = sizeof(FlutterLogicalKeyEvent),
    .kind = kind,
    .key = logicalKey,
    .character_size = 0,
  };
  FlutterLogicalKeyEvent logical_events[] = {logical_event};
  FlutterKeyEvent flutterEvent = {
    .struct_size = sizeof(FlutterKeyEvent),
    .logical_event_count = 1,
    .logical_events = logical_events,
    .logical_characters_data = kEmptyCharacterData,
    .timestamp = timestamp,
    .lockFlags = _lockFlags,
    .kind = kind,
    .key = physicalKey,
    .repeated = false,
  };
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
}

- (void)dispatchDownEvent:(NSEvent*)event {
  uint64_t physicalKey = GetPhysicalKeyForEvent(event);
  uint64_t logicalKey = GetLogicalKeyForEvent(event, physicalKey);

  //=== Calculate logical events ===
  // The event corresponding to `event` (in contrast to `cancel_logical_event`)
  FlutterLogicalKeyEvent core_logical_event = {
    .struct_size = sizeof(FlutterLogicalKeyEvent),
  };
  // A possible event to cancel the previous event, used to switch logical keys
  // being pressed.
  FlutterLogicalKeyEvent cancel_logical_event = {
    .struct_size = sizeof(FlutterLogicalKeyEvent),
  };
  size_t character_size;
  uint8_t logical_characters_data[kMaxCharactersSize];
  // Whether this event is a repeated event with a different logical key, indicating
  // events that update the logical key needs to be dispatched.
  BOOL switchingLogical = NO;

  NSNumber* pressedLogicalKey = _pressedKeys[@(physicalKey)];
  character_size = [event.characters lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  if (pressedLogicalKey) {
    if (event.isARepeat) {
      if (pressedLogicalKey.unsignedLongLongValue != logicalKey) {
        // Switching logical keys
        switchingLogical = YES;
        cancel_logical_event.kind = kFlutterKeyEventKindCancel;
        cancel_logical_event.key = logicalKey;
        cancel_logical_event.character_size = 0;
        cancel_logical_event.repeated = false;
        core_logical_event.kind = kFlutterKeyEventKindSync;
        core_logical_event.key = pressedLogicalKey.unsignedLongLongValue;
        core_logical_event.character_size = character_size;
        core_logical_event.repeated = false;
      } else {
        // Regular repeated down event
        core_logical_event.kind = kFlutterKeyEventKindDown;
        core_logical_event.key = logicalKey;
        core_logical_event.character_size = character_size;
        core_logical_event.repeated = true;
      }
    } else {
      // A non-repeated key has been pressed that has the exact physical key
      // as a currently pressed one, usually indicating multiple keyboards are
      // pressing keys with the same physical key. The down event is ignored.
      return;
    }
  } else { // pressedLogicalKey is null
    if (event.isARepeat) {
      // A repeated key has been pressed that has no record of being pressed.
      // This indicates multiple keyboards are pressing keys with the same
      // physical key, and the first key has been released. Ignore the pressing state
      // of the 2nd key.
      return;
    } else {
      // New down event
      core_logical_event.kind = kFlutterKeyEventKindDown;
      core_logical_event.key = logicalKey;
      core_logical_event.character_size = character_size;
      core_logical_event.repeated = false;
    }
  }
  [self updateKey:physicalKey asPressed:logicalKey];
  NSAssert((character_size < kMaxCharactersSize),
      @"Unexpected long key characters: |%@|. Please report this to Flutter.", event.characters);
  memcpy(logical_characters_data, event.characters.UTF8String, character_size);

  //=== Build physical event ===
  FlutterLogicalKeyEvent logical_events[2];
  uint8_t logical_event_count = 0;
  if (switchingLogical) {
    logical_events[logical_event_count++] = cancel_logical_event;
  }
  logical_events[logical_event_count++] = core_logical_event;

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .logical_event_count = logical_event_count,
      .logical_events = logical_events,
      .logical_characters_data = logical_characters_data,
      .timestamp = GetFlutterTimestampFrom(event),
      .lockFlags = _lockFlags,
      .kind = kFlutterKeyEventKindDown,
      .key = physicalKey,
      .repeated = (event.isARepeat != NO),
  };
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
}

- (void)dispatchUpEvent:(NSEvent*)event {
  NSAssert(!event.isARepeat,
      @"Unexpected repeated Up event. Please report this to Flutter.", event.characters);

  uint64_t physicalKey = GetPhysicalKeyForEvent(event);
  NSNumber* pressedLogicalKey = _pressedKeys[@(physicalKey)];
  if (!pressedLogicalKey) {
    // The physical key has been released before. It indicates multiple
    // keyboards pressed keys with the same physical key. Ignore the up event.
    return;
  }
  [self updateKey:physicalKey asPressed:0];
  [self sendSimpleEventWithKind: kFlutterKeyEventKindUp
                    physicalKey: physicalKey
                     logicalKey: pressedLogicalKey.unsignedLongLongValue
                      timestamp: GetFlutterTimestampFrom(event)];
}

- (void)dispatchCapsLockEvent:(NSEvent*)event {
  bool lockIsOn = (event.modifierFlags & NSEventModifierFlagCapsLock) != 0;
  [self updateLockFlag:kFlutterKeyLockFlagCapsLock asOn:lockIsOn];
  double timestamp = GetFlutterTimestampFrom(event);
  // MacOS sends a Down or an Up when CapsLock is pressed, depending on whether
  // the lock is enabled or disabled. This event should always be converted to
  // a Down and a cancel, since we don't know how long it will be pressed.
  [self sendSimpleEventWithKind: kFlutterKeyEventKindDown
                    physicalKey: kCapsLockPhysicalKey
                     logicalKey: GetLogicalKeyForModifier(kCapsLockPhysicalKey)
                      timestamp: timestamp];
  [self sendSimpleEventWithKind: kFlutterKeyEventKindCancel
                    physicalKey: kCapsLockPhysicalKey
                     logicalKey: GetLogicalKeyForModifier(kCapsLockPhysicalKey)
                      timestamp: timestamp];
}

// Process events where modifier keys are pressed or released.
- (void)dispatchFlagEvent:(NSEvent*)event {
  // NSEvent only tells us the key that triggered the event and the resulting
  // flag, but not whether the change is a down or up. For keys such as
  // CapsLock, the change type can be inferred from the key and the flag.
  //
  // But some other modifier keys come in paris, such as shift or control, and
  // both of the pair share one flag, which indicates either key is pressed.
  // If the pressing states of paired keys are desynchronized due to loss of
  // focus, the change might have to be guessed and synchronized.
  //
  // For convenience, the key corresponding to `event.keycode` is called
  // `targetKey`. If the key is among a pair, the other key is called
  // `siblingKey`.
  //
  // The logic of guessing is shown as follows, based on whether the key pairs
  // were recorded as pressed and whether the incoming flag is set: ("*"
  // indicates some extent of synthesizing.)
  //
  // LastPressed \ NowEither |                   |
  //  (Tgt,Sbl)   \  Pressed |         1         |          0
  //  -----------------------|-------------------|-------------------
  //                (1, 0)   | *       -         |        TgtUp
  //                (0, 0)   |      TgtDown      | *        -
  //                (0, 1)   |      TgtDown      | *    SblCancel
  //                (1, 1)   |       TgtUp       | * SblCancel,TgtUp
  //
  // For non-pair keys, lastSiblingPressed is always set to 0, resulting in the
  // top half of the table.

  uint64_t targetKey = GetPhysicalKeyForEvent(event);
  if (targetKey == kCapsLockPhysicalKey) {
    return [self dispatchCapsLockEvent:event];
  }
  uint64_t modifierFlag = GetModifierFlagForKey(targetKey);
  if (targetKey == 0 || modifierFlag == 0) {
    // Unrecognized modifier.
    return;
  }
  // The `siblingKey` may be 0, which means it doesn't have a sibling key.
  uint64_t siblingKey = GetSiblingKeyForKey(targetKey);

  bool lastTargetPressed = [_pressedKeys objectForKey:@(targetKey)] != nil;
  bool lastSiblingPressed = siblingKey == 0 ? false : [_pressedKeys objectForKey:@(siblingKey)] != nil;
  bool nowEitherPressed = (event.modifierFlags & modifierFlag) != 0;

  bool targetKeyShouldDown = !lastTargetPressed && nowEitherPressed;
  bool targetKeyShouldUp = lastTargetPressed && (lastSiblingPressed || !nowEitherPressed);
  bool siblingKeyShouldCancel = lastSiblingPressed && !nowEitherPressed;

  double timestamp = GetFlutterTimestampFrom(event);
  if (siblingKeyShouldCancel) {
    [self updateKey:siblingKey asPressed:0];
    [self sendSimpleEventWithKind: kFlutterKeyEventKindCancel
                      physicalKey: siblingKey
                       logicalKey: GetLogicalKeyForModifier(siblingKey)
                        timestamp: timestamp];
  }

  if (targetKeyShouldDown || targetKeyShouldUp) {
    uint64_t logicalKey = GetLogicalKeyForModifier(targetKey);
    [self updateKey:targetKey asPressed:(targetKeyShouldDown ? logicalKey : 0)];
    [self sendSimpleEventWithKind: targetKeyShouldDown ? kFlutterKeyEventKindDown : kFlutterKeyEventKindUp
                      physicalKey: targetKey
                       logicalKey: logicalKey
                        timestamp: timestamp];
  }
}

- (void)dispatchEvent:(NSEvent*)event {
  switch (event.type) {
    case NSEventTypeKeyDown:
      [self dispatchDownEvent:event];
      break;
    case NSEventTypeKeyUp:
      [self dispatchUpEvent:event];
      break;
    case NSEventTypeFlagsChanged:
      [self dispatchFlagEvent:event];
      break;
    default:
      NSAssert(false, @"Unexpected key event type: |%@|.", @(event.type));
  }

}

#pragma mark - Private


@end
