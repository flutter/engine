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

- (instancetype)initWithViewController:(FlutterViewController*)viewController;

- (void)dispatchEvent:(NSEvent*)event;

@end
