// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

@interface FlutterKeyboardManager ()

/**
 * TODO
 */
@property(nonatomic, weak) NSResponder* owner;

/**
 * TODO
 */
@property(nonatomic) NSMutableArray<id<FlutterKeyHandler>>* keyHandlers;

/**
 * A list of additional responders to keyboard events.
 *
 * Keyboard events received by FlutterViewController are first dispatched to
 * each additional responder in order. If any of them handle the event (by
 * returning true), the event is not dispatched to later additional responders
 * or to the nextResponder.
 */
@property(nonatomic) NSMutableArray<id<FlutterKeyFinalResponder>>* additionalKeyHandlers;

@end

@implementation FlutterKeyboardManager

- (nonnull instancetype)initWithOwner:(NSResponder*)weakOwner {
  self = [super init];
  _owner = weakOwner;
  _keyHandlers = [[NSMutableArray alloc] init];
  _additionalKeyHandlers = [[NSMutableArray alloc] init];
  return self;
}

- (void)addHandler:(nonnull id<FlutterKeyHandler>)handler {
  [_keyHandlers addObject:handler];
}

- (void)addAdditionalHandler:(nonnull id<FlutterKeyFinalResponder>)handler {
  [_additionalKeyHandlers addObject:handler];
}

- (void)dispatchToAdditionalHandlers:(NSEvent*)event {
  for (id<FlutterKeyFinalResponder> responder in _additionalKeyHandlers) {
    if ([responder handleKeyEvent:event]) {
      return;
    }
  }
  switch (event.type) {
    case NSEventTypeKeyDown:
      if ([_owner.nextResponder respondsToSelector:@selector(keyDown:)]) {
        [_owner.nextResponder keyDown:event];
      }
      break;
    case NSEventTypeKeyUp:
      if ([_owner.nextResponder respondsToSelector:@selector(keyUp:)]) {
        [_owner.nextResponder keyUp:event];
      }
      break;
    case NSEventTypeFlagsChanged:
      if ([_owner.nextResponder respondsToSelector:@selector(flagsChanged:)]) {
        [_owner.nextResponder flagsChanged:event];
      }
      break;
    default:
      NSAssert(false, @"Unexpected key event type (got %lu).", event.type);
  }
}

- (void)handleEvent:(nonnull NSEvent*)event {
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
      [weakSelf dispatchToAdditionalHandlers:event];
    }
  };

  for (id<FlutterKeyHandler> handler in _keyHandlers) {
    [handler handleEvent:event callback:replyCallback];
  }
}

@end
