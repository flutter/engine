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

/**
 * The FlutterEngine class coordinates a single instance of execution for a
 * `FlutterDartProject`.  It may have one `FlutterViewController` at any given
 * time, which can be `-setViewController:` or with `FlutterViewController`'s 
 * `initWithEngine` initializer.
 * 
 * A FlutterEngine can be created independently of a `FlutterViewController` for
 * headless execution.  It can also persist across the lifespan of multiple 
 * `FlutterViewController` instances to maintain state and/or asynchronous tasks
 * (such as downloading a large file).
 * 
 * Alternatively, you can simply create a new `FlutterViewController` with only a
 * `FlutterDartProject`. That `FlutterViewController` will internally manage its 
 * own instance of a FlutterEngine, but will not guarantee survival of the engine
 * beyond the life of the ViewController.
 */
FLUTTER_EXPORT
@interface FlutterEngine
    : NSObject<FlutterBinaryMessenger, FlutterTextureRegistry, FlutterPluginRegistry>
/**
 * Iniitalize this FlutterEngine with a `FlutterDartProject`.
 *
 * If the FlutterDartProject is not specified, the FlutterEngine will attempt to locate
 * the project in a default location.
 *
 * @param labelPrefix The label prefix used to identify threads for this instance. Should
 * be unique across FlutterEngine instances
 * @param project The `FlutterDartProject` to run.
 */
- (instancetype)initWithName:(NSString*)labelPrefix
                     project:(FlutterDartProject*)projectOrNil NS_DESIGNATED_INITIALIZER;

/**
 * The default initializer is not available for this object.
 * Callers must use `-[FlutterEngine initWithName:project:]`.
 */
- (instancetype)init NS_UNAVAILABLE;

/**
 * Runs a Dart program on an Isolate.
 *
 * The first call will create a new Isolate. Subsequent calls will return immediately.
 *
 * @param entrypoint The name of a top-level function from the same Dart
 *   library that contains the app's main() function.
 * @return YES if the call succeeds in creating and running a Flutter Engine instance; NO otherwise.
 */
- (bool)runWithEntrypoint:(NSString*)entrypoint;

/**
* Runs a Dart program on an Isolate.
* 
* The first call will create a new Isolate. Subsequent calls will return immediately.
* 
* @param entrypoint The name of a top-level function from a Dart library.
* @param libraryUri The URI of the Dart library which contains the entrypoint method.
* @return YES if the call succeeds in creating and running a Flutter Engine instance; NO otherwise.
*/
- (bool)runWithEntrypointAndLibraryUri:(NSString*)entrypoint libraryUri:(NSString*)uri;

/**
 * Sets the `FlutterViewController` for this instance; callers may pass nil to
 * remove the viewController and have the engine run headless in the current process.
 * 
 * A FlutterEngine can only have one `FlutterViewController` at a time. If there is 
 * already a `FlutterViewController` associated with this instance, this method will replace
 * the engine's current viewController with the newly specified one.
 * 
 * @param viewController The `FlutterViewController` (or nil) to set. Kept as a weak reference.
 */
- (void)setViewController:(FlutterViewController*)viewController;
/**
 * Gets the `FlutterViewController` (may be nil) currently used by this instance.
 */
- (FlutterViewController*)getViewController;

/**
 * The `FlutterMethodChannel` used for localization related platform messages, such as
 * setting the locale.
 */
- (FlutterMethodChannel*)localizationChannel;
/**
 * The `FlutterMethodChannel` used for navigation related platform messages.
 * 
 * @see [Navigation Channel](https://docs.flutter.io/flutter/services/SystemChannels/navigation-constant.html)
 * @see [Navigator Widget](https://docs.flutter.io/flutter/widgets/Navigator-class.html)
 */
- (FlutterMethodChannel*)navigationChannel;
/**
 * The `FlutterMethodChannel` used for core platform messages, such as
 * information about the screen orientation.
 */
- (FlutterMethodChannel*)platformChannel;
/**
 * The `FlutterMethodChannel` used to communicate text input events to the
 * Dart Isolate.
 * 
 * @see [Text Input Channel](https://docs.flutter.io/flutter/services/SystemChannels/textInput-constant.html)
 */
- (FlutterMethodChannel*)textInputChannel;
/**
 * The `FlutterBasicMessageChannel` used to communicate app lifecycle events
 * to the Dart Isolate.
 * 
 * @see [Lifecycle Channel](https://docs.flutter.io/flutter/services/SystemChannels/lifecycle-constant.html)
 */
- (FlutterBasicMessageChannel*)lifecycleChannel;
/**
 * The `FlutterBasicMessageChannel` used for communicating system events, such as
 * memory pressure events.
 * 
 * @see [System Channel](https://docs.flutter.io/flutter/services/SystemChannels/system-constant.html)
 */
- (FlutterBasicMessageChannel*)systemChannel;
/**
 * The `FlutterBasicMessageChannel` used for communicating user settings such as
 * clock format and text scale.
 */
- (FlutterBasicMessageChannel*)settingsChannel;

@end

#endif  // FLUTTER_FLUTTERENGINE_H_
