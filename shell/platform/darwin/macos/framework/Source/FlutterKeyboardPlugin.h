// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"

/**
 * A plugin to handle hardward keyboard.
 *
 * Responsible for bridging the native macOS key event system with the
 * Flutter framework hardware key event classes, via embedder.
 */
@interface FlutterKeyboardPlugin : NSObject

- (nonnull instancetype)initWithViewController:(nonnull FlutterViewController*)viewController;

/**
 * Handles the method call that activates a system cursor.
 *
 * Returns a FlutterError if the arguments can not be recognized. Otherwise
 * returns nil.
 */
- (void)dispatchEvent:(nonnull NSEvent*)event;

@end
