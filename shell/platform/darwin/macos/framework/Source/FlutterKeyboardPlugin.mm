// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/embedder/embedder.h"
#import "FlutterKeyboardPlugin.h"
#import "KeyCodeMap_internal.h"

// Values of `characterIgnoringModifiers` that must not be directly converted to
// a logical key value, but should look up `keyCodeToLogical` by `keyCode`.
// This is because each of these of character codes is mapped from multiple
// logical keys, usually because of numpads.
// static const NSSet* kAmbiguousCharacters = @{
// }

// Whether a string represents a control character.
static bool IsControlCharacter(NSUInteger length, NSString *label) {
  if (length > 1) {
    return false;
  }
  unichar codeUnit = [label characterAtIndex:0];
  return (codeUnit <= 0x1f && codeUnit >= 0x00) || (codeUnit >= 0x7f && codeUnit <= 0x9f);
}

// Whether a string represents an unprintable key.
static bool IsUnprintableKey(NSUInteger length, NSString *label) {
  if (length > 1) {
    return false;
  }
  unichar codeUnit = [label characterAtIndex:0];
  return codeUnit >= 0xF700 && codeUnit <= 0xF8FF;
}

// Returns a key code composited by a base key and a plane.
static uint64_t KeyOfPlane(uint64_t baseKey, uint64_t plane) {
  return plane | (baseKey & kValueMask);
}

// Find the sibling key for a physical key by looking up `siblingKeyCodes`.
//
// Returns 0 if not found.
static uint64_t GetSiblingKeyCodeForKey(uint64_t physicalKey) {
  NSNumber* siblingKey = [siblingKeyCodes objectForKey:@(physicalKey)];
  if (siblingKey == nil)
    return 0;
  return siblingKey.unsignedLongLongValue;
}

// Find the modifer flag for a physical key by looking up `modiferFlags`.
//
// Returns 0 if not found.
static uint64_t GetModifierFlagForKey(uint64_t physicalKey) {
  NSNumber* modifierFlag = [modiferFlags objectForKey:@(physicalKey)];
  if (modifierFlag == nil)
    return 0;
  return modifierFlag.unsignedLongLongValue;
}

// Returns the physical key for a key code.
static uint64_t GetPhysicalKeyForKeyCode(uint64_t keyCode) {
  NSNumber* physicalKeyKey = [keyCodeToPhysicalKey objectForKey:@(keyCode)];
  if (physicalKeyKey == nil)
    return 0;
  return physicalKeyKey.unsignedLongLongValue;
}

// Returns the logical key for a modifier physical key.
static uint64_t GetLogicalKeyForModifier(uint64_t keyCode, uint64_t hidCode) {
  NSNumber* fromKeyCode = [keyCodeToLogicalKey objectForKey:@(keyCode)];
  if (fromKeyCode != nil)
    return fromKeyCode.unsignedLongLongValue;
  return KeyOfPlane(hidCode, kHidPlane);
}

// Returns the logical key of a KeyUp or KeyDown event.
//
// For FlagsChanged event, use GetLogicalKeyForModifier.
static uint64_t GetLogicalKeyForEvent(NSEvent* event, uint64_t physicalKey) {
  // Look to see if the keyCode can be mapped from keycode.
  NSNumber* fromKeyCode = [keyCodeToLogicalKey objectForKey:@(event.keyCode)];
  if (fromKeyCode != nil)
    return fromKeyCode.unsignedLongLongValue;

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
    if (codeUnit < 256) {
      if (isupper(codeUnit)) {
        return tolower(codeUnit);
      }
      return codeUnit;
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

// Returns the timestamp for an event.
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
@property(nonatomic) NSMutableDictionary* pressingRecords;

/**
 * Update the pressing state.
 *
 * If `logicalKey` is not 0, `physicalKey` is pressed as `logicalKey`.
 * Otherwise, `physicalKey` is released.
 */
- (void)updateKey:(uint64_t)physicalKey asPressed:(uint64_t)logicalKey;

/**
 * Processes a down event.
 */
- (void)dispatchDownEvent:(NSEvent*)event;

/**
 * Processes an up event.
 */
- (void)dispatchUpEvent:(NSEvent*)event;

/**
 * Processes a flags changed event, where modifier keys are pressed or released.
 */
- (void)dispatchFlagEvent:(NSEvent*)event;

/**
 * Processes all kinds of events.
 */
- (bool)dispatchEvent:(NSEvent*)event;

@end

@implementation FlutterKeyboardPlugin

- (instancetype)initWithViewController:(FlutterViewController*)viewController {
  self = [super init];
  if (self != nil) {
    _flutterViewController = viewController;
    _pressingRecords = [NSMutableDictionary dictionary];
  }
  return self;
}

- (void)updateKey:(uint64_t)physicalKey asPressed:(uint64_t)logicalKey {
  if (logicalKey == 0) {
    [_pressingRecords removeObjectForKey:@(physicalKey)];
  } else {
    _pressingRecords[@(physicalKey)] = @(logicalKey);
  }
}

- (void)dispatchDownEvent:(NSEvent*)event {
  printf("Down event keyCode %d cIM %s c %s\n", [event keyCode], [[event charactersIgnoringModifiers] UTF8String], [[event characters] UTF8String]);
  uint64_t physicalKey = GetPhysicalKeyForKeyCode(event.keyCode);
  uint64_t logicalKey = GetLogicalKeyForEvent(event, physicalKey);

  NSNumber* pressedLogicalKey = _pressingRecords[@(physicalKey)];
  bool isARepeat = false;
  bool isSynthesized = false;

  if (pressedLogicalKey) {
    // This physical key is being pressed according to the record.
    if (event.isARepeat) {
      // A normal repeated key.
      isARepeat = true;
    } else {
      // A non-repeated key has been pressed that has the exact physical key as
      // a currently pressed one, usually indicating multiple keyboards are
      // pressing keys with the same physical key, or the up event was lost
      // during a loss of focus. The down event is ignored.
      return;
    }
  } else {
    // This physical key is not being pressed according to the record. It's a
    // normal down event, whether the system event is a repeat or not.
  }
  if (pressedLogicalKey == nil) {
    [self updateKey:physicalKey asPressed:logicalKey];
  }

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = GetFlutterTimestampFrom(event),
      .kind = isARepeat ? kFlutterKeyEventKindRepeat : kFlutterKeyEventKindDown,
      .physical = physicalKey,
      .logical = pressedLogicalKey == nil ? logicalKey : [pressedLogicalKey unsignedLongLongValue],
      .character = event.characters.UTF8String,
      .synthesized = isSynthesized,
  };
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
}

- (void)dispatchUpEvent:(NSEvent*)event {
  NSAssert(!event.isARepeat,
      @"Unexpected repeated Up event. Please report this to Flutter.", event.characters);

  uint64_t physicalKey = GetPhysicalKeyForKeyCode(event.keyCode);
  NSNumber* pressedLogicalKey = _pressingRecords[@(physicalKey)];
  if (!pressedLogicalKey) {
    // The physical key has been released before. It indicates multiple
    // keyboards pressed keys with the same physical key. Ignore the up event.
    return;
  }
  [self updateKey:physicalKey asPressed:0];

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = GetFlutterTimestampFrom(event),
      .kind = kFlutterKeyEventKindUp,
      .physical = physicalKey,
      .logical = [pressedLogicalKey unsignedLongLongValue],
      .character = nil,
      .synthesized = false,
  };
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
}

- (void)dispatchCapsLockEvent:(NSEvent*)event {
  NSNumber* logicalKey = [keyCodeToLogicalKey objectForKey:@(event.keyCode)];
  if (logicalKey == nil)
    return;
  uint64_t logical = logicalKey.unsignedLongLongValue;

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = GetFlutterTimestampFrom(event),
      .kind = kFlutterKeyEventKindDown,
      .physical = kCapsLockPhysicalKey,
      .logical = logical,
      .character = nil,
      .synthesized = false,
  };
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];

  // MacOS sends a Down or an Up when CapsLock is pressed, depending on whether
  // the lock is enabled or disabled. This event should always be converted to
  // a Down and a cancel, since we don't know how long it will be pressed.
  flutterEvent.kind = kFlutterKeyEventKindUp;
  flutterEvent.synthesized = true;
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
}

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
  // indicates synthesized event.)
  //
  // LastPressed \ NowEither |                   |
  //  (Tgt,Sbl)   \  Pressed |         1         |          0
  //  -----------------------|-------------------|-------------------
  //                (1, 0)   |         -         |        TgtUp
  //                (0, 0)   |      TgtDown      |          -
  //                (0, 1)   |      TgtDown      |        SblUp*
  //                (1, 1)   |       TgtUp       |     SblUp*,TgtUp
  //
  // For non-pair keys, lastSiblingPressed is always set to 0, resulting in the
  // top half of the table.

  uint64_t targetKey = GetPhysicalKeyForKeyCode(event.keyCode);
  if (targetKey == kCapsLockPhysicalKey) {
    return [self dispatchCapsLockEvent:event];
  }
  uint64_t modifierFlag = GetModifierFlagForKey(targetKey);
  if (targetKey == 0 || modifierFlag == 0) {
    // Unrecognized modifier.
    return;
  }
  // The `siblingKeyCode` may be 0, which means it doesn't have a sibling key.
  uint64_t siblingKeyCode = GetSiblingKeyCodeForKey(event.keyCode);
  uint64_t siblingKeyPhysical = siblingKeyCode == 0 ? 0 : GetPhysicalKeyForKeyCode(siblingKeyCode);
  uint64_t siblingKeyLogical = siblingKeyCode == 0 ? 0 : GetLogicalKeyForModifier(siblingKeyCode, siblingKeyPhysical);

  bool lastTargetPressed = [_pressingRecords objectForKey:@(targetKey)] != nil;
  bool lastSiblingPressed = siblingKeyCode == 0 ? false : [_pressingRecords objectForKey:@(siblingKeyPhysical)] != nil;
  bool nowEitherPressed = (event.modifierFlags & modifierFlag) != 0;

  bool targetKeyShouldDown = !lastTargetPressed && nowEitherPressed;
  bool targetKeyShouldUp = lastTargetPressed && (lastSiblingPressed || !nowEitherPressed);
  bool siblingKeyShouldUp = lastSiblingPressed && !nowEitherPressed;

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = GetFlutterTimestampFrom(event),
      .character = nil,
  };
  if (siblingKeyShouldUp) {
    flutterEvent.kind = kFlutterKeyEventKindUp;
    flutterEvent.physical = siblingKeyPhysical;
    flutterEvent.logical = siblingKeyLogical;
    flutterEvent.synthesized = true;
    [self updateKey:siblingKeyPhysical asPressed:0];
    [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
  }

  if (targetKeyShouldDown || targetKeyShouldUp) {
    uint64_t logicalKey = GetLogicalKeyForModifier(event.keyCode, targetKey);
    flutterEvent.kind = targetKeyShouldDown ? kFlutterKeyEventKindDown : kFlutterKeyEventKindUp;
    flutterEvent.physical = targetKey;
    flutterEvent.logical = logicalKey;
    flutterEvent.synthesized = false;
    [self updateKey:targetKey asPressed:(targetKeyShouldDown ? logicalKey : 0)];
    [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
  }
}

- (bool)dispatchEvent:(NSEvent*)event {
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

  return true; // TODO
}

#pragma mark - Private


@end
