// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERHEADLESSDARTRUNNER_H_
#define FLUTTER_FLUTTERHEADLESSDARTRUNNER_H_

#import <Foundation/Foundation.h>

#include "FlutterBinaryMessenger.h"
#include "FlutterDartProject.h"
#include "FlutterEngine.h"
#include "FlutterMacros.h"

/**
 * A callback for when FlutterHeadlessDartRunner has attempted to start a Dart
 * Isolate in the background.
 *
 * @param success YES if the Isolate was started and run successfully, NO
 *   otherwise.
 */
typedef void (^FlutterHeadlessDartRunnerCallback)(BOOL success);

/**
 * The FlutterHeadlessDartRunner runs Flutter Dart code with a null rasterizer,
 * and no native drawing surface. It is appropriate for use in running Dart
 * code e.g. in the background from a plugin.
 *
 * Most callers should prefer using `FlutterEngine` directly; this interface exists
 * for legacy support.
 */
FLUTTER_EXPORT
FLUTTER_DEPRECATED("FlutterEngine should be used rather than FlutterHeadlessDartRunner")
@interface FlutterHeadlessDartRunner : FlutterEngine

@end

#endif  // FLUTTER_FLUTTERHEADLESSDARTRUNNER_H_
