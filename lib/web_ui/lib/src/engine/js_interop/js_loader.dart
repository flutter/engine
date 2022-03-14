// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@JS()
library js_loader;

import 'package:js/js.dart';

import 'js_promise.dart';

// FlutterEngineInitializer

/// Base-class for both types of EngineInitializers
@JS()
@anonymous
abstract class FlutterEngineInitializer{
  external factory FlutterEngineInitializer({
    required InitializeEngineFn initializeEngine,
  });
}

/// A class that exposes a function that continues initializing the engine,
/// and returns the promise of a FlutterAppInitializer.
@JS()
@anonymous
abstract class CustomFlutterEngineInitializer extends FlutterEngineInitializer {
  /// Creates an instance of [CustomFlutterEngineInitializer].
  external factory CustomFlutterEngineInitializer({
    required InitializeEngineFn initializeEngine,
  });
}

@JS()
@anonymous
abstract class AutomaticFlutterEngineInitializer extends FlutterEngineInitializer {
  external factory AutomaticFlutterEngineInitializer({
    required InitializeEngineFn initializeEngine,
  });
}

/// The shape of the object that can be passed as parameter to the
/// initializeEngine function of the FlutterEngineInitializer object
/// (when called from JS).
@JS()
@anonymous
abstract class InitializeEngineFnParameters {
}

/// Typedef for the function that initializes the flutter engine.
typedef InitializeEngineFn = Promise<FlutterAppRunner?> Function(InitializeEngineFnParameters?);

// FlutterAppRunner

/// A class that exposes a function that runs the Flutter app,
/// and returns a promise of a FlutterAppCleaner.
@JS()
@anonymous
abstract class FlutterAppRunner {
  /// Runs a flutter app
  external factory FlutterAppRunner({
    required RunAppFn runApp, // Returns an App
  });
}

/// The shape of the object that can be passed as parameter to the
/// runApp function of the FlutterAppRunner object (from JS).
@JS()
@anonymous
abstract class RunAppFnParameters {
}

/// Typedef for the function that runs the flutter app main entrypoint.
typedef RunAppFn = Promise<FlutterAppCleaner> Function(RunAppFnParameters?);

// FlutterAppCleaner

/// A class that exposes a function that cleans up the App that is currently
/// running. Returns a promise of a boolean value, which resolves to true if
/// the app has been cleaned up successfully.
@JS()
@anonymous
abstract class FlutterAppCleaner {
  /// Cleans a Flutter app
  external factory FlutterAppCleaner({
    required CleanAppFn cleanApp,
  });
}

/// The shape of the object that can be passed as parameter to the
/// cleanApp function of the FlutterAppRunner object (from JS).
@JS()
@anonymous
abstract class CleanAppFnParameters {
}

/// Typedef for the function that cleans the flutter app initialized by the functions above.
typedef CleanAppFn = Promise<bool> Function (CleanAppFnParameters?);
