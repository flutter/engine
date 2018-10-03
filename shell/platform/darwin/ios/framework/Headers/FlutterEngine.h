// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERENGINE_H_
#define FLUTTER_FLUTTERENGINE_H_

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "FlutterBinaryMessenger.h"
#include "FlutterDartProject.h"
#include "FlutterMacros.h"
#include "FlutterPlugin.h"
#include "FlutterTexture.h"

@class FlutterViewController;

FLUTTER_EXPORT
@interface FlutterEngine
    : NSObject<FlutterBinaryMessenger, FlutterTextureRegistry, FlutterPluginRegistry>
/**
 Iniitalize this FlutterEngine with a FlutterDartProject.
*/
- (instancetype)initWithName:(NSString*)labelPrefix
                  andProject:(FlutterDartProject*)projectOrNil NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

- (void)launchEngine;

/**
 Runs a Dart function on an Isolate.
 The first call will create a new Isolate. Subsequent calls will return
 immediately.

 - Parameter entrypoint: The name of a top-level function from the same Dart
   library that contains the app's main() function.
*/
- (bool)runWithEntrypoint:(NSString*)entrypoint;

/**
 Runs a Dart function on an Isolate.
 The first call will create a new Isolate. Subsequent calls will return
 immediately.

 - Parameter entrypoint: The name of a top-level function from a Dart library.
 - Parameter uri: The URI of the Dart library which contains entrypoint.
*/
- (bool)runWithEntrypointAndLibraryUri:(NSString*)entrypoint libraryUri:(NSString*)uri;

- (void)setViewController:(FlutterViewController*)viewController;
- (FlutterViewController*)getViewController;

- (FlutterMethodChannel*)localizationChannel;
- (FlutterMethodChannel*)navigationChannel;
- (FlutterMethodChannel*)platformChannel;
- (FlutterMethodChannel*)textInputChannel;
- (FlutterBasicMessageChannel*)lifecycleChannel;
- (FlutterBasicMessageChannel*)systemChannel;
- (FlutterBasicMessageChannel*)settingsChannel;

@end

#endif  // FLUTTER_FLUTTERENGINE_H_
