// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@JS()
library js_loader;

import 'package:js/js.dart';

import 'js_promise.dart';

/// Typedef for the function that notifies JS that the main entrypoint is up and running.
/// As a parameter, a [FlutterEngineInitializer] instance is passed to JS, so the
/// programmer can control the initialization sequence.
typedef DidLoadMainDartJsFn = void Function(FlutterEngineInitializer);

/// A hook to notify JavaScript that Flutter is up and running!
/// This is setup by flutter.js when the main entrypoint bundle is injected.
@JS('_flutter.loader.didLoadMainDartJs')
external DidLoadMainDartJsFn? get didLoadMainDartJs;

// FlutterEngineInitializer

/// Base-class for both types of EngineInitializers
@JS()
@anonymous
abstract class FlutterEngineInitializer{
  external factory FlutterEngineInitializer({
    required InitializeEngineFn initializeEngine,
    required ImmediateRunAppFn autoStart,
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
typedef InitializeEngineFn = Promise<FlutterAppRunner?> Function([InitializeEngineFnParameters?]);

/// Typedef for the `autoStart` function that can be called straight from an engine initializer instance.
/// (Similar to [RunAppFn], but taking no specific "runApp" parameters).
typedef ImmediateRunAppFn = Promise<FlutterApp> Function();

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
typedef RunAppFn = Promise<FlutterApp> Function([RunAppFnParameters?]);

// FlutterApp

/// A class that exposes the public API of a running Flutter Web App running.
@JS()
@anonymous
abstract class FlutterApp {
  /// Cleans a Flutter app
  external factory FlutterApp();
}
