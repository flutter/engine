// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyboardManager.h"

@interface FlutterKeyboardManager ()

/**
 * The owner set by initWithOwner.
 */
@property(nonatomic, weak) NSResponder* owner;

/**
 * The primary responders added by addPrimaryResponder.
 */
@property(nonatomic) NSMutableArray<id<FlutterKeyPrimaryResponder>>* keyHandlers;

/**
 * The secondary responders added by addSecondaryResponder.
 */
@property(nonatomic) NSMutableArray<id<FlutterKeySecondaryResponder>>* additionalKeyHandlers;

@end

@implementation FlutterKeyboardManager

- (nonnull instancetype)initWithOwner:(NSResponder*)weakOwner {
  self = [super init];
  _owner = weakOwner;
  _keyHandlers = [[NSMutableArray alloc] init];
  _additionalKeyHandlers = [[NSMutableArray alloc] init];
  return self;
}

- (void)addPrimaryResponder:(nonnull id<FlutterKeyPrimaryResponder>)handler {
  [_keyHandlers addObject:handler];
}

- (void)addSecondaryResponder:(nonnull id<FlutterKeySecondaryResponder>)handler {
  [_additionalKeyHandlers addObject:handler];
}

- (void)dispatchToAdditionalHandlers:(NSEvent*)event {
  for (id<FlutterKeySecondaryResponder> responder in _additionalKeyHandlers) {
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
  // Be sure to add a handling method in propagateKeyEvent if you allow more
  // event types here.
  if (event.type != NSEventTypeKeyDown && event.type != NSEventTypeKeyUp &&
      event.type != NSEventTypeFlagsChanged) {
    return;
  }
  // Having no primary responders require extra logic, but since Flutter adds
  // all primary responders in hard-code, this is a situation that Flutter will
  // never meet.
  NSAssert([_keyHandlers count] >= 0, @"At least one primary responder must be added.");

  __weak __typeof__(self) weakSelf = self;
  __block int unreplied = [_keyHandlers count];
  __block BOOL anyHandled = false;
  FlutterAsyncKeyCallback replyCallback = ^(BOOL handled) {
    unreplied -= 1;
    NSAssert(unreplied >= 0, @"More primary responders replied than possible.");
    anyHandled = anyHandled || handled;
    if (unreplied == 0 && !anyHandled) {
      [weakSelf dispatchToAdditionalHandlers:event];
    }
  };

  for (id<FlutterKeyPrimaryResponder> handler in _keyHandlers) {
    [handler handleEvent:event callback:replyCallback];
  }
}

@end
