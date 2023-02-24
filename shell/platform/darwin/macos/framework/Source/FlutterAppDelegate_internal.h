// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERAPPDELEGATE_INTERNAL_H_
#define FLUTTER_FLUTTERAPPDELEGATE_INTERNAL_H_

#include "embedder.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppDelegate.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"

@interface FlutterAppDelegate ()
/**
 * Allows the engine to add a handler for termination requests
 */
- (void)setTerminationRequestHandler:(FlutterEngineTerminationHandler*)handler;

/**
 * Allows the delegate to respond to an attempt to terminate the application.
 */
- (void)requestApplicationTermination:(NSApplication*)application exitType:(FlutterAppExitType)type;
@end

#endif  // FLUTTER_FLUTTERAPPDELEGATE_INTERNAL_H_
