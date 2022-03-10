// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

/**
 * An interface for a class that can provides |FlutterKeyboardManager| with
 * platform-related features.
 *
 * This protocol is typically implemented by |FlutterViewController|.
 */
@protocol FlutterKeyboardViewDelegate

@required

/**
 * Get the next responder to dispatch events that the keyboard system
 * (including text input) do not handle.
 *
 * If the |nextResponder| is null, then those events will be discarded.
 */
@property(nonatomic, readonly, nullable) NSResponder* nextResponder;

/**
 * Dispatch events that are not handled by the keyboard event handlers
 * to the text input handler.
 *
 * This method typically forwards events to |TextInputPlugin.handleKeyEvent|.
 */
- (BOOL)onTextInputKeyEvent:(nonnull NSEvent*)event;

@end
