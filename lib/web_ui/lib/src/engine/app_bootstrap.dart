// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js.dart';

import 'js_interop/js_loader.dart';
import 'js_interop/js_promise.dart';

/// A class that controls the coarse lifecycle of a Flutter app.
class AppBootstrap {
  /// Construct a FlutterLoader
  AppBootstrap({Function? initEngine, Function? runApp, Function? cleanApp}) :
    _initEngine = initEngine, _runApp = runApp, _cleanApp = cleanApp;

  // TODO(dit): Be more strict with the below typedefs, so we can add incoming params for each function.

  // A function to initialize the engine
  final Function? _initEngine;

  // A function to run the app
  final Function? _runApp;

  // A function to clear the app (optional)
  final Function? _cleanApp;

  // Calls a function that may be null and/or async, and wraps it in a Future<Object?>
  Future<Object?> _safeCall(Function? fn) async {
    if (fn != null) {
      // ignore: avoid_dynamic_calls
      return Future<Object?>.value(fn());
    }
    return null;
  }

  /// Immediately bootstraps the app.
  ///
  /// This calls `initEngine` and `runApp` in succession.
  Future<void> now() async {
    await _safeCall(_initEngine);
    await _safeCall(_runApp);
  }

  /// Creates an engineInitializer that runs our encapsulated initEngine function.
  FlutterEngineInitializer prepareCustomEngineInitializer() {
    // Return an object that has a initEngine method...
    return FlutterEngineInitializer(
      // initEngine and runApp in one call. Does not return any lifecycle object,
      // nor accepts any incoming parameters. This is a convenience method that
      // implements something similar to the "autoStart" mode, but triggered
      // from from JavaScript.
      runApp: allowInterop(now),
      // Return a JS Promise that resolves to an AppRunner object.
      initializeEngine: allowInterop(([InitializeEngineFnParameters? params]) {
        // `params` coming from Javascript may be used to configure the engine intialization.
        // The internal `initEngine` function must accept those params, and then this
        // code needs to be slightly modified to pass them to the initEngine call below.
        return Promise<FlutterAppRunner>(allowInterop((
          PromiseResolver<FlutterAppRunner> resolve,
          PromiseRejecter _,
        ) async {
          await _safeCall(_initEngine);
          // Resolve with an actual AppRunner object, created in a similar way
          // to how the FlutterEngineInitializer was created
          resolve(_prepareAppRunner());
        }));
      }),
    );
  }

  /// Creates an appRunner that runs our encapsulated runApp function.
  FlutterAppRunner _prepareAppRunner() {
    return FlutterAppRunner(runApp: allowInterop(([RunAppFnParameters? params]) {
      // `params` coming from JS may be used to configure the run app method.
      return Promise<FlutterAppCleaner>(allowInterop((
        PromiseResolver<FlutterAppCleaner> resolve,
        PromiseRejecter _,
      ) async {
        await _safeCall(_runApp);
        // Next step is an AppCleaner
        resolve(_prepareAppCleaner());
      }));
    }));
  }

  /// Clean the app that was injected above.
  FlutterAppCleaner _prepareAppCleaner() {
    return FlutterAppCleaner(
        cleanApp: allowInterop(([CleanAppFnParameters? params]) {
      return Promise<bool>(allowInterop((
        PromiseResolver<bool> resolve,
        PromiseRejecter _,
      ) async {
        await _safeCall(_cleanApp);
        resolve(true);
      }));
    }));
  }
}
