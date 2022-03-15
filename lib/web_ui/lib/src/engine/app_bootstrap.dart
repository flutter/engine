// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js.dart';

import 'js_interop/js_loader.dart';
import 'js_interop/js_promise.dart';

/// A class that controls the coarse lifecycle of a Flutter app.
class AppBootstrap {
  /// Construct a FlutterLoader
  AppBootstrap({this.initEngine, this.runApp, this.cleanApp});

  // TODO(dit): Be more strict with the below typedefs, so we can add incoming params for each function.
  // TODO(dit): Make the below private members.

  /// A function to initialize the engine
  final Function? initEngine;

  /// A function to run the app
  final Function? runApp;

  /// A function to clear the app (optional)
  final Function? cleanApp;

  // Calls a function that may be null and/or async, and wraps it in a Future<Object?>
  Future<Object?> _safeCall(Function? fn) async {
    if (fn != null) {
      // ignore: avoid_dynamic_calls
      return Future<Object?>.value(fn());
    }
    return null;
  }

  /// Immediately bootstraps the app
  Future<void> now() async {
    await _safeCall(initEngine);
    await _safeCall(runApp);
  }

  /// Creates an engineInitializer that runs our encapsulated initEngine function.
  FlutterEngineInitializer prepareCustomEngineInitializer() {
    // Return an object that has a initEngine method...
    return FlutterEngineInitializer(
      initializeEngine: allowInterop((InitializeEngineFnParameters? params) {
        // `params` coming from Javascript may be used to configure the engine intialization.
        // The internal `initEngine` function must accept those params, and then this
        // code needs to be slightly modified to pass them to the initEngine call below.
        return Promise<FlutterAppRunner>(allowInterop((
          PromiseResolver<FlutterAppRunner> resolve,
          PromiseRejecter _,
        ) async {
          await _safeCall(initEngine);
          // Resolve with an actual AppRunner object, created in a similar way
          // to how the FlutterEngineInitializer was created
          resolve(_prepareAppRunner());
        }));
      }),
      // initEngine and runApp in one call. Does not return any lifecycle object,
      // nor accepts any incoming parameters. This implements the "legacy" way of
      // bootstrapping an app.
      runApp: allowInterop(() {
        return Promise<void>(allowInterop((
          PromiseResolver<void> resolve,
          PromiseRejecter _,
        ) async {
          await now();
          resolve(null);
        }));
      }),
    );
  }

  /// Creates an appRunner that runs our encapsulated runApp function.
  FlutterAppRunner _prepareAppRunner() {
    return FlutterAppRunner(runApp: allowInterop((RunAppFnParameters? params) {
      // `params` coming from JS may be used to configure the run app method.
      return Promise<FlutterAppCleaner>(allowInterop((
        PromiseResolver<FlutterAppCleaner> resolve,
        PromiseRejecter _,
      ) async {
        await _safeCall(runApp);
        // Next step is an AppCleaner
        resolve(_prepareAppCleaner());
      }));
    }));
  }

  /// Clean the app that was injected above.
  FlutterAppCleaner _prepareAppCleaner() {
    return FlutterAppCleaner(
        cleanApp: allowInterop((CleanAppFnParameters? params) {
      return Promise<bool>(allowInterop((
        PromiseResolver<bool> resolve,
        PromiseRejecter _,
      ) async {
        await _safeCall(cleanApp);
        resolve(true);
      }));
    }));
  }
}
