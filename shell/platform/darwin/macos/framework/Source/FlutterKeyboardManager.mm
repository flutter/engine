// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

namespace {
}

@interface FlutterKeyboardManager ()

/**
 * TODO
 */
@property(nonatomic, weak) NSResponder* owner;

/**
 * TODO
 */
@property(nonatomic) NSMutableArray<id<FlutterKeyHandlerBase>>* keyHandlers;

/**
 * A list of additional responders to keyboard events.
 *
 * Keyboard events received by FlutterViewController are first dispatched to
 * each additional responder in order. If any of them handle the event (by
 * returning true), the event is not dispatched to later additional responders
 * or to the nextResponder.
 */
@property(nonatomic) NSMutableArray<FlutterIntermediateKeyResponder*>* additionalKeyHandlers;

/**
 * The current state of the keyboard and pressed keys.
 */
@property(nonatomic) uint64_t previouslyPressedFlags;

- (void)dispatchKeyEvent:(NSEvent*)event ofType:(NSString*)type;

@end

@implementation FlutterKeyboardManager

- (nonnull instancetype)initWithOwner:(NSResponder*)weakOwner {
  self = [super init];
  _owner = weakOwner;
  _keyHandlers = [[NSMutableArray alloc] init];
  _additionalKeyHandlers = [[NSMutableArray alloc] init];
  _previouslyPressedFlags = 0;
  return self;
}

- (void)addHandler:(nonnull id<FlutterKeyHandlerBase>)handler {
  [_keyHandlers addObject:handler];
}

- (void)addAdditionalHandler:(nonnull FlutterIntermediateKeyResponder*)handler {
  [_additionalKeyHandlers addObject:handler];
}

- (void)dispatchToAdditionalHandlers:(NSEvent*)event ofType:(NSString*)type {
  if ([type isEqual:@"keydown"]) {
    for (FlutterIntermediateKeyResponder* responder in _additionalKeyHandlers) {
      if ([responder handleKeyDown:event]) {
        return;
      }
    }
    if ([_owner.nextResponder respondsToSelector:@selector(keyDown:)] &&
        event.type == NSEventTypeKeyDown) {
      [_owner.nextResponder keyDown:event];
    }
  } else if ([type isEqual:@"keyup"]) {
    for (FlutterIntermediateKeyResponder* responder in _additionalKeyHandlers) {
      if ([responder handleKeyUp:event]) {
        return;
      }
    }
    if ([_owner.nextResponder respondsToSelector:@selector(keyUp:)] &&
        event.type == NSEventTypeKeyUp) {
      [_owner.nextResponder keyUp:event];
    }
  }
  if ([_owner.nextResponder respondsToSelector:@selector(flagsChanged:)] &&
      event.type == NSEventTypeFlagsChanged) {
    [_owner.nextResponder flagsChanged:event];
  }
}

- (void)dispatchKeyEvent:(NSEvent*)event ofType:(NSString*)type {
  // Be sure to add a handler in propagateKeyEvent if you allow more event
  // types here.
  if (event.type != NSEventTypeKeyDown && event.type != NSEventTypeKeyUp &&
      event.type != NSEventTypeFlagsChanged) {
    return;
  }

  __weak __typeof__(self) weakSelf = self;
  __block int unreplied = [_keyHandlers count];
  __block BOOL anyHandled = false;
  FlutterKeyHandlerCallback replyCallback = ^(BOOL handled) {
    unreplied -= 1;
    NSAssert(unreplied >= 0, @"More key handlers replied than intended.");
    anyHandled = anyHandled || handled;
    if (unreplied == 0 && !anyHandled) {
      [weakSelf dispatchToAdditionalHandlers:event ofType:type];
    }
  };

  for (id<FlutterKeyHandlerBase> handler in _keyHandlers) {
    [handler handleEvent:event ofType:type callback:replyCallback];
  }
}

- (void)keyDown:(nonnull NSEvent*)event {
  [self dispatchKeyEvent:event ofType:@"keydown"];
}

- (void)keyUp:(nonnull NSEvent*)event {
  [self dispatchKeyEvent:event ofType:@"keyup"];
}

- (void)flagsChanged:(NSEvent*)event {
  if (event.modifierFlags < _previouslyPressedFlags) {
    [self keyUp:event];
  } else {
    [self keyDown:event];
  }
  _previouslyPressedFlags = event.modifierFlags;
}

@end
