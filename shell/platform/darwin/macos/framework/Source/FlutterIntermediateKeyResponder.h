// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

/*
 * An interface for adding key responders that are processed after the framework
 * has had a chance to handle the key, but before the next responder is given
 * the key.
 *
 * It differs from an NSResponder in that it returns a boolean from the keyUp
 * and keyDown calls, allowing the callee to indicate whether or not it handled
 * the given event. If handled, then the event is not passed to the next
 * responder.
 */
@interface FlutterIntermediateKeyResponder : NSObject
/*
 * Informs the receiver that the user has released a key.
 */
- (BOOL)handleKeyUp:(NSEvent*)event;
/*
 * Informs the receiver that the user has pressed a key.
 */
- (BOOL)handleKeyDown:(NSEvent*)event;
@end
