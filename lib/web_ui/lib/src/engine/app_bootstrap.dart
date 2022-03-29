// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js.dart';

import 'js_interop/js_loader.dart';
import 'js_interop/js_promise.dart';

/// A class that controls the coarse lifecycle of a Flutter app.
class AppBootstrap {
  /// Construct a FlutterLoader
  AppBootstrap({required Function initEngine, required Function runApp}) :
    _initEngine = initEngine, _runApp = runApp;

  // TODO(dit): Be more strict with the below typedefs, so we can add incoming params for each function.

  // A function to initialize the engine
  final Function _initEngine;

  // A function to run the app
  final Function _runApp;

  /// Immediately bootstraps the app.
  ///
  /// This calls `initEngine` and `runApp` in succession.
  Future<void> autoStart() async {
    await _initEngine();
    await _runApp();
  }

  /// Creates an engineInitializer that runs our encapsulated initEngine function.
  FlutterEngineInitializer prepareEngineInitializer() {
    // Return an object that has a initEngine method...
    return FlutterEngineInitializer(
      // initEngine and runApp in one call. Returns the running App, but does not
      // accept any incoming parameters. This is a convenience method that
      // implements something similar to the "autoStart" mode, but triggered
      // from from JavaScript.
      autoStart: allowInterop(() {
        return Promise<FlutterApp>(allowInterop((
          PromiseResolver<FlutterApp> resolve,
          PromiseRejecter _,
        ) async {
          await autoStart();
          // Return the App that was just started
          resolve(_prepareFlutterApp());
        }));
      }),
      // Return a JS Promise that resolves to an AppRunner object.
      initializeEngine: allowInterop(([InitializeEngineFnParameters? params]) {
        // `params` coming from Javascript may be used to configure the engine intialization.
        // The internal `initEngine` function must accept those params, and then this
        // code needs to be slightly modified to pass them to the initEngine call below.
        return Promise<FlutterAppRunner>(allowInterop((
          PromiseResolver<FlutterAppRunner> resolve,
          PromiseRejecter _,
        ) async {
          await _initEngine();
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
      return Promise<FlutterApp>(allowInterop((
        PromiseResolver<FlutterApp> resolve,
        PromiseRejecter _,
      ) async {
        await _runApp();
        // Return the App that was just started
        resolve(_prepareFlutterApp());
      }));
    }));
  }

  /// Represents the App that was just started, and its JS API.
  FlutterApp _prepareFlutterApp() {
    return FlutterApp();
  }
}
