// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <AppKit/AppKit.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterPluginMacOS.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"

/**
 * A plugin to configure and control the native system menu.
 *
 * Responsible for bridging the native macOS menu system with the Flutter
 * framework menu system classes, via system channels.
 */
@interface FlutterMenuPlugin : NSObject <FlutterPlugin>

/**
 * Handles the method call that sets the current menu configuration.
 *
 * Returns a FlutterError if the arguments can not be recognized. Otherwise
 * returns nil.
 */
- (void)setMenu:(nonnull NSArray*)arguments;

/**
 * Handles all method calls for the menu channel from Flutter.
 */
- (void)handleMethodCall:(nonnull FlutterMethodCall*)call result:(nonnull FlutterResult)result;
@end
