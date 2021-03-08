// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

/*
 * An interface for a key responder that can declare itself as the final
 * responder of the event, terminating the event propagation.
 */
@protocol FlutterKeyFinalResponder
/*
 * Informs the receiver that the user has interacted with a key.
 *
 * The return value indicates whether it has handled the given event.
 *
 * Default implementation returns NO.
 */
@required
- (BOOL)handleKeyEvent:(NSEvent*)event;
@end
