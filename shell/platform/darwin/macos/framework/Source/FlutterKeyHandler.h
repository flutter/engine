// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

typedef void (^FlutterKeyHandlerCallback)(BOOL handled);

/**
 * An interface for a responder that can process a key event and decides whether
 * to handle an event asynchronously.
 *
 * To use this class, add it to a |FlutterKeyboardManager| with |addHandler|.
 */
@protocol FlutterKeyHandler

/**
 * Process the event.
 *
 * The |callback| should be called with a value that indicates whether the
 * handler has handled the given event. The |callback| must be called exactly
 * once, and can be called before the return of this method, or after.
 */
@required
- (void)handleEvent:(nonnull NSEvent*)event callback:(nonnull FlutterKeyHandlerCallback)callback;

@end
