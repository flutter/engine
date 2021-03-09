// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandler.h"

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyFinalResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandler.h"

/**
 * A hub that manages how key events are dispatched to various handlers, and
 * whether the event is propagated to the next responder.
 *
 * This class can be added with one or more handlers, as well as zero or more
 * additional handlers.
 *
 * An event that is received by |handleEvent| is first dispatched to *all*
 * handlers. Each handler responds ascynchronously with a boolean, indicating
 * whether it handles the event.
 *
 * An event that is not handled by any handlers, is then passed to to the first
 * added additional handlers, which responds synchronously with a boolean,
 * indicating whether it handles the event. If not, the event is passed to the
 * next additional responder, and so on.
 *
 * If all handlers and additional handlers respond with not to handle the
 * event, the event is then passed to the owner's |nextResponder| if not nil,
 * on method |keyDown|, |keyUp|, or |flagsChanged|, depending on the event's
 * type. If the |nextResponder| is nil, then the event will be propagated no
 * further.
 *
 * Preventing handlers from receiving events is not supported, because
 * in reality this class will only support 2 hardcoded delegates (channel and
 * embedder), where the only purpose of supporting two is to support the legacy API (channel) during
 * the deprecation window, after which the channel handler should be removed.
 */
@interface FlutterKeyboardManager : NSObject

/**
 * Create a manager by specifying the owner.
 *
 * The owner should be an object that handles the lifecycle of this instance.
 * The |owner.nextResponder| can be nil, but if it isn't, it will be where the
 * key events are propagated to, if no handlers or additional handlers handle
 * the event. The owner is typically a |FlutterViewController|.
 */
- (nonnull instancetype)initWithOwner:(nonnull NSResponder*)weakOwner;

/**
 * Add a handler, which asynchronously decides whether to handle an event.
 */
- (void)addHandler:(nonnull id<FlutterKeyHandler>)handler;

/**
 * Add an additional handler, which synchronously decides whether to handle an
 * event.
 */
- (void)addAdditionalHandler:(nonnull id<FlutterKeyFinalResponder>)handler;

/**
 * Dispatch a key event to all handlers and additional handlers, and possibly
 * the next responder afterwards.
 */
- (void)handleEvent:(nonnull NSEvent*)event;

@end
