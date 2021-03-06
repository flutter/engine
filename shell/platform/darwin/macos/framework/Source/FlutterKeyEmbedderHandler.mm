// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "FlutterKeyEmbedderHandler.h"
#import "KeyCodeMap_internal.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/embedder/embedder.h"

// Values of `charactersIgnoringModifiers` that must not be directly converted to
// a logical key value, but should look up `keyCodeToLogical` by `keyCode`.
// This is because each of these of character codes is mapped from multiple
// logical keys, usually because of numpads.
// static const NSSet* kAmbiguousCharacters = @{
// }

namespace {

// Isolate the least significant 1-bit.
//
// For example,
//
//  * lowestSetBit(0x1010) returns 0x10.
//  * lowestSetBit(0) returns 0.
static NSUInteger lowestSetBit(NSUInteger bitmask) {
  return bitmask & -bitmask;
}

// Whether a string represents a control character.
static bool IsControlCharacter(NSUInteger length, NSString* label) {
  if (length > 1) {
    return false;
  }
  unichar codeUnit = [label characterAtIndex:0];
  return (codeUnit <= 0x1f && codeUnit >= 0x00) || (codeUnit >= 0x7f && codeUnit <= 0x9f);
}

// Whether a string represents an unprintable key.
static bool IsUnprintableKey(NSUInteger length, NSString* label) {
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

// Find the modifier flag for a physical key by looking up `modifierFlags`.
//
// Returns 0 if not found.
// static uint64_t GetModifierFlagForKey(uint64_t physicalKey) {
//   NSNumber* modifierFlag = [keyCodeToModifierFlag objectForKey:@(physicalKey)];
//   if (modifierFlag == nil)
//     return 0;
//   return modifierFlag.unsignedLongLongValue;
// }

// Returns the physical key for a key code.
static uint64_t GetPhysicalKeyForKeyCode(unsigned short keyCode) {
  NSNumber* physicalKeyKey = [keyCodeToPhysicalKey objectForKey:@(keyCode)];
  if (physicalKeyKey == nil)
    return 0;
  return physicalKeyKey.unsignedLongLongValue;
}

// Returns the logical key for a modifier physical key.
static uint64_t GetLogicalKeyForModifier(unsigned short keyCode, uint64_t hidCode) {
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
  if (keyLabelLength != 0 && !IsControlCharacter(keyLabelLength, keyLabel) &&
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

static NSUInteger computeModifierFlagOfInterestMask() {
  __block NSUInteger modifierFlagOfInterestMask = NSEventModifierFlagCapsLock;
  [keyCodeToModifierFlag
      enumerateKeysAndObjectsUsingBlock:^(NSNumber* keyCode, NSNumber* flag, BOOL* stop) {
        modifierFlagOfInterestMask = modifierFlagOfInterestMask | [flag unsignedLongValue];
      }];
  return modifierFlagOfInterestMask;
}

void HandleResponse(bool handled, void* user_data);

// Converts NSEvent.characters to a C-string for FlutterKeyEvent.
const char* getEventString(NSString* characters) {
  if ([characters length] == 0) {
    return nullptr;
  }
  unichar utf16Code = [characters characterAtIndex:0];
  if (utf16Code >= 0xf700 && utf16Code <= 0xf7ff) {
    // Some function keys are assigned characters with codepoints from the
    // private use area (see
    // https://developer.apple.com/documentation/appkit/1535851-function-key_unicodes?language=objc).
    // These characters are filtered out since they're unprintable.
    //
    // Although the documentation claims to reserve 0xF700-0xF8FF, only up to 0xF747
    // is actually used. Here we choose to filter out 0xF700-0xF7FF section.
    // The reason for keeping the 0xF800-0xF8FF section is because 0xF8FF is
    // used for the "Apple logo" character (Option-Shift-K on US keyboard.)
    return nullptr;
  }
  return [characters UTF8String];
}
}  // namespace

/* An entry of FlutterKeyEmbedderHandler.pendingResponse.
 *
 * FlutterEngineSendKeyEvent only supports a global function and a pointer.
 * This class is used for the global function, HandleResponse, to convert a
 * pointer into a method call, FlutterKeyEmbedderHandler.handleResponse.
 */
@interface FlutterKeyPendingResponse : NSObject

@property(nonatomic) FlutterKeyEmbedderHandler* handler;

@property(nonatomic) uint64_t responseId;

- (nonnull instancetype)initWithHandler:(nonnull FlutterKeyEmbedderHandler*)handler
                             responseId:(uint64_t)responseId;

@end

@implementation FlutterKeyPendingResponse
- (instancetype)initWithHandler:(FlutterKeyEmbedderHandler*)handler
                     responseId:(uint64_t)responseId {
  self = [super init];
  _handler = handler;
  _responseId = responseId;
  return self;
}
@end

@interface FlutterKeyEmbedderHandler ()

/**
 *
 */
@property(nonatomic, copy) FlutterSendEmbedderKeyEvent sendEvent;

/**
 * A map of presessd keys.
 *
 * The keys of the dictionary are physical keys, while the values are the logical keys
 * on pressing.
 */
@property(nonatomic) NSMutableDictionary<NSNumber*, NSNumber*>* pressingRecords;

@property(nonatomic) NSUInteger lastModifierFlags;

/**
 * A constant mask for NSEvent.modifierFlags that Flutter tries to keep
 * synchronized on.
 *
 * It equals to the sum of all values of keyCodeToModifierFlag as well as
 * NSEventModifierFlagCapsLock.
 */
@property(nonatomic) NSUInteger modifierFlagOfInterestMask;

@property(nonatomic) uint64_t responseId;

@property(nonatomic) NSMutableDictionary<NSNumber*, FlutterKeyHandlerCallback>* pendingResponses;

- (void)synchronizeModifiers:(NSUInteger)currentFlags ignoringFlags:(NSUInteger)ignoringFlags;

/**
 * Update the pressing state.
 *
 * If `logicalKey` is not 0, `physicalKey` is pressed as `logicalKey`.
 * Otherwise, `physicalKey` is released.
 */
- (void)updateKey:(uint64_t)physicalKey asPressed:(uint64_t)logicalKey;

- (void)sendPrimaryFlutterEvent:(const FlutterKeyEvent&)event
                       callback:(nonnull FlutterKeyHandlerCallback)callback;

/**
 * Send a CapsLock down event, then a CapsLock up event.
 *
 * If downCallback is nil, then both events will be synthesized. Otherwise, the
 * downCallback will be used as the callback for the down event, which is not
 * synthesized.
 */
- (void)sendCapsLockTapWithCallback:(nullable FlutterKeyHandlerCallback)downCallback;

- (void)sendModifierEventForDown:(BOOL)shouldDown
                         keyCode:(unsigned short)keyCode
                        callback:(nullable FlutterKeyHandlerCallback)downCallback;

/**
 * Processes a down event.
 */
- (void)handleDownEvent:(nonnull NSEvent*)event
               callback:(nonnull FlutterKeyHandlerCallback)callback;

/**
 * Processes an up event.
 */
- (void)handleUpEvent:(nonnull NSEvent*)event callback:(nonnull FlutterKeyHandlerCallback)callback;

/**
 * Processes an up event.
 */
- (void)handleCapsLockEvent:(nonnull NSEvent*)event
                   callback:(nonnull FlutterKeyHandlerCallback)callback;

/**
 * Processes a flags changed event, where modifier keys are pressed or released.
 */
- (void)handleFlagEvent:(nonnull NSEvent*)event
               callback:(nonnull FlutterKeyHandlerCallback)callback;

- (void)handleResponse:(BOOL)handled forId:(uint64_t)responseId;

@end

@implementation FlutterKeyEmbedderHandler

- (nonnull instancetype)initWithSendEvent:(FlutterSendEmbedderKeyEvent)sendEvent {
  self = [super init];
  if (self != nil) {
    _sendEvent = sendEvent;
    _pressingRecords = [NSMutableDictionary dictionary];
    _pendingResponses = [NSMutableDictionary dictionary];
    _responseId = 1;
    _lastModifierFlags = 0;
    _modifierFlagOfInterestMask = computeModifierFlagOfInterestMask();
  }
  return self;
}

#pragma mark - Private

- (void)synchronizeModifiers:(NSUInteger)currentFlags ignoringFlags:(NSUInteger)ignoringFlags {
  const NSUInteger updatingMask = _modifierFlagOfInterestMask & ~ignoringFlags;
  const NSUInteger currentInterestedFlags = currentFlags & updatingMask;
  const NSUInteger lastInterestedFlags = _lastModifierFlags & updatingMask;
  NSUInteger flagDifference = currentInterestedFlags ^ lastInterestedFlags;
  if (flagDifference & NSEventModifierFlagCapsLock) {
    [self sendCapsLockTapWithCallback:nil];
    flagDifference = flagDifference & ~NSEventModifierFlagCapsLock;
  }
  while (true) {
    const NSUInteger currentFlag = lowestSetBit(flagDifference);
    if (currentFlag == 0) {
      break;
    }
    flagDifference = flagDifference & ~currentFlag;
    NSNumber* keyCode = [modifierFlagToKeyCode objectForKey:@(currentFlag)];
    NSAssert(keyCode != nil, @"Invalid modifier flag 0x%lx", currentFlag);
    if (keyCode == nil) {
      continue;
    }
    BOOL shouldDown = (currentInterestedFlags & currentFlag) != 0;
    [self sendModifierEventForDown:shouldDown keyCode:[keyCode unsignedShortValue] callback:nil];
  }
  _lastModifierFlags = (_lastModifierFlags & ~updatingMask) | currentInterestedFlags;
}

- (void)updateKey:(uint64_t)physicalKey asPressed:(uint64_t)logicalKey {
  if (logicalKey == 0) {
    [_pressingRecords removeObjectForKey:@(physicalKey)];
  } else {
    _pressingRecords[@(physicalKey)] = @(logicalKey);
  }
}

- (void)sendPrimaryFlutterEvent:(const FlutterKeyEvent&)event
                       callback:(FlutterKeyHandlerCallback)callback {
  _responseId += 1;
  uint64_t responseId = _responseId;
  FlutterKeyPendingResponse* pending =
      [[FlutterKeyPendingResponse alloc] initWithHandler:self responseId:responseId];
  _pendingResponses[@(responseId)] = callback;
  // The `__bridge_retained` here is matched by `__bridge_transfer` in HandleResponse.
  _sendEvent(event, HandleResponse, (__bridge_retained void*)pending);
}

- (void)sendCapsLockTapWithCallback:(FlutterKeyHandlerCallback)downCallback {
  // MacOS sends a down *or* an up when CapsLock is tapped, alternatively on
  // even taps and odd taps. A CapsLock down or CapsLock up should always be
  // converted to a down *and* an up, and the up should always be a synthesized
  // event, since we will never know when the button is released.
  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = 0,  // TODO
      .type = kFlutterKeyEventTypeDown,
      .physical = kCapsLockPhysicalKey,
      .logical = kCapsLockLogicalKey,
      .character = nil,
      .synthesized = downCallback == nil,
  };
  if (downCallback != nil) {
    [self sendPrimaryFlutterEvent:flutterEvent callback:downCallback];
  } else {
    _sendEvent(flutterEvent, nullptr, nullptr);
  }

  flutterEvent.type = kFlutterKeyEventTypeUp;
  flutterEvent.synthesized = true;
  _sendEvent(flutterEvent, nullptr, nullptr);
}

- (void)sendModifierEventForDown:(BOOL)shouldDown
                         keyCode:(unsigned short)keyCode
                        callback:(FlutterKeyHandlerCallback)callback {
  uint64_t physicalKey = GetPhysicalKeyForKeyCode(keyCode);
  uint64_t logicalKey = GetLogicalKeyForModifier(keyCode, physicalKey);
  if (physicalKey == 0 || logicalKey == 0) {
    NSLog(@"Unrecognized modifier key: keyCode 0x%hx, physical key 0x%llx", keyCode, physicalKey);
    callback(TRUE);
    return;
  }
  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = 0,  // TODO
      .type = shouldDown ? kFlutterKeyEventTypeDown : kFlutterKeyEventTypeUp,
      .physical = physicalKey,
      .logical = logicalKey,
      .character = nil,
      .synthesized = callback == nil,
  };
  [self updateKey:physicalKey asPressed:shouldDown ? logicalKey : 0];
  if (callback != nil) {
    [self sendPrimaryFlutterEvent:flutterEvent callback:callback];
  } else {
    _sendEvent(flutterEvent, nullptr, nullptr);
  }
}

- (void)handleDownEvent:(NSEvent*)event callback:(FlutterKeyHandlerCallback)callback {
  uint64_t physicalKey = GetPhysicalKeyForKeyCode(event.keyCode);
  uint64_t logicalKey = GetLogicalKeyForEvent(event, physicalKey);
  [self synchronizeModifiers:event.modifierFlags ignoringFlags:0];

  NSNumber* pressedLogicalKey = _pressingRecords[@(physicalKey)];
  bool isARepeat = (pressedLogicalKey != nil) || event.isARepeat;
  bool isSynthesized = false;

  if (pressedLogicalKey == nil) {
    [self updateKey:physicalKey asPressed:logicalKey];
  }

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = GetFlutterTimestampFrom(event),
      .type = isARepeat ? kFlutterKeyEventTypeRepeat : kFlutterKeyEventTypeDown,
      .physical = physicalKey,
      .logical = pressedLogicalKey == nil ? logicalKey : [pressedLogicalKey unsignedLongLongValue],
      .character = getEventString(event.characters),
      .synthesized = isSynthesized,
  };
  [self sendPrimaryFlutterEvent:flutterEvent callback:callback];
}

- (void)handleUpEvent:(NSEvent*)event callback:(FlutterKeyHandlerCallback)callback {
  NSAssert(!event.isARepeat, @"Unexpected repeated Up event: keyCode %d, char %@, charIM %@",
           event.keyCode, event.characters, event.charactersIgnoringModifiers);
  [self synchronizeModifiers:event.modifierFlags ignoringFlags:0];

  uint64_t physicalKey = GetPhysicalKeyForKeyCode(event.keyCode);
  NSNumber* pressedLogicalKey = _pressingRecords[@(physicalKey)];
  if (pressedLogicalKey == nil) {
    NSAssert(FALSE,
             @"Received key up event that has not been pressed: "
             @"keyCode %d, char %@, charIM %@, previously pressed logical 0x%llx",
             event.keyCode, event.characters, event.charactersIgnoringModifiers,
             [pressedLogicalKey unsignedLongLongValue]);
    callback(TRUE);
    return;
  }
  [self updateKey:physicalKey asPressed:0];

  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .timestamp = GetFlutterTimestampFrom(event),
      .type = kFlutterKeyEventTypeUp,
      .physical = physicalKey,
      .logical = [pressedLogicalKey unsignedLongLongValue],
      .character = nil,
      .synthesized = false,
  };
  [self sendPrimaryFlutterEvent:flutterEvent callback:callback];
}

- (void)handleCapsLockEvent:(NSEvent*)event callback:(FlutterKeyHandlerCallback)callback {
  [self synchronizeModifiers:event.modifierFlags ignoringFlags:NSEventModifierFlagCapsLock];
  if ((_lastModifierFlags & NSEventModifierFlagCapsLock) !=
      (event.modifierFlags & NSEventModifierFlagCapsLock)) {
    [self sendCapsLockTapWithCallback:callback];
    _lastModifierFlags = _lastModifierFlags ^ NSEventModifierFlagCapsLock;
  } else {
    callback(TRUE);
  }
}

- (void)handleFlagEvent:(NSEvent*)event callback:(FlutterKeyHandlerCallback)callback {
  NSNumber* targetModifierFlagObj = keyCodeToModifierFlag[@(event.keyCode)];
  NSUInteger targetModifierFlag =
      targetModifierFlagObj == nil ? 0 : [targetModifierFlagObj unsignedLongValue];
  uint64_t targetKey = GetPhysicalKeyForKeyCode(event.keyCode);
  if (targetKey == kCapsLockPhysicalKey) {
    return [self handleCapsLockEvent:event callback:callback];
  }

  [self synchronizeModifiers:event.modifierFlags ignoringFlags:targetModifierFlag];

  NSNumber* pressedLogicalKey = [_pressingRecords objectForKey:@(targetKey)];
  BOOL lastTargetPressed = pressedLogicalKey != nil;
  NSAssert(targetModifierFlagObj == nil ||
               (_lastModifierFlags & targetModifierFlag) != 0 == lastTargetPressed,
           @"Desynchronized state between lastModifierFlags (0x%lx) on bit 0x%lx "
           @"for keyCode 0x%hx, whose pressing state is %@.",
           _lastModifierFlags, targetModifierFlag, event.keyCode,
           lastTargetPressed
               ? [NSString stringWithFormat:@"0x%llx", [pressedLogicalKey unsignedLongLongValue]]
               : @"empty");
  BOOL shouldBePressed = (event.modifierFlags & targetModifierFlag) != 0;
  printf("Key %hx lastP %d shouldP %d\n", event.keyCode, lastTargetPressed, shouldBePressed);

  if (lastTargetPressed == shouldBePressed) {
    callback(TRUE);
    return;
  }
  _lastModifierFlags = _lastModifierFlags ^ targetModifierFlag;
  [self sendModifierEventForDown:shouldBePressed keyCode:event.keyCode callback:callback];
}

void ps(const char* s) {
  int i = 0;
  while (s[i] != 0) {
    printf("%02hhx", s[i]);
    i++;
  }
  if (i == 0)
    printf("00");
}

- (void)handleEvent:(NSEvent*)event callback:(FlutterKeyHandlerCallback)callback {
  printf("#### Event %d keyCode 0x%hx mod 0x%lx ", (int)event.type, event.keyCode,
         event.modifierFlags);
  if (event.type != NSEventTypeFlagsChanged) {
    printf("rep %d cIM %s(", event.isARepeat, [[event charactersIgnoringModifiers] UTF8String]);
    ps([[event charactersIgnoringModifiers] UTF8String]);
    printf(") c %s(", [[event characters] UTF8String]);
    ps([[event characters] UTF8String]);
    printf(")\n");
  } else {
    printf("\n");
  }
  // The conversion algorithm relies on a non-nil callback to properly compute
  // `synthesized`. If someday callback is allowed to be nil, make a dummy empty
  // callback instead.
  NSAssert(callback != nil, @"The callback must not be nil.");
  switch (event.type) {
    case NSEventTypeKeyDown:
      [self handleDownEvent:event callback:callback];
      break;
    case NSEventTypeKeyUp:
      [self handleUpEvent:event callback:callback];
      break;
    case NSEventTypeFlagsChanged:
      [self handleFlagEvent:event callback:callback];
      break;
    default:
      NSAssert(false, @"Unexpected key event type: |%@|.", @(event.type));
  }
  NSAssert(_lastModifierFlags == (event.modifierFlags & _modifierFlagOfInterestMask),
           @"The modifier flags are not properly updated: recorded 0x%lx, event 0x%lx",
           _lastModifierFlags, (event.modifierFlags & _modifierFlagOfInterestMask));
}

- (void)handleResponse:(BOOL)handled forId:(uint64_t)responseId {
  printf("HR %d %llu\n", handled, responseId);
  FlutterKeyHandlerCallback callback = _pendingResponses[@(responseId)];
  callback(handled);
  printf("after\n");
  [_pendingResponses removeObjectForKey:@(responseId)];
}

@end

namespace {
void HandleResponse(bool handled, void* user_data) {
  printf("Resp %d %0llx\n", handled, (uint64_t)user_data);
  // The `__bridge_transfer` here is matched by `__bridge_retained` in sendPrimaryFlutterEvent.
  FlutterKeyPendingResponse* pending = (__bridge_transfer FlutterKeyPendingResponse*)user_data;
  [pending.handler handleResponse:handled forId:pending.responseId];
}
}  // namespace
