// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';

/// Performs a full initialization of the web environment that supports the
/// Flutter framework.
///
/// Unlike [warmupEngine], this method initializes UI and non-UI services.
Future<void> initializePlatform() async {
  await initializeEngine();
}

/// Initializes essential bits of the engine before it fully initializes.
///
/// When [FlutterLoaderExtension.didCreateEngineInitializer] is set, it delegates
/// engine initialization and app startup to the programmer. Else, it immediately
/// triggers the full engine + app bootstrap.
///
/// This method is called by the flutter_tools package, from the entrypoint that
/// it generates around the main method provided by the programmer. See:
/// * https://github.com/flutter/flutter/blob/95be76ab7e3dca2def54454313e97f94f4ac4582/packages/flutter_tools/lib/src/web/file_generators/main_dart.dart#L14-L43
///
/// This function first calls [initializeEngineServices] so the engine can
/// prepare its non-UI services. It then creates a JsObject that is passed to
/// the [FlutterLoaderExtension.didCreateEngineInitializer] JS callback, to
/// delegate bootstrapping the app to the programmer.
///
/// If said callback is not defined, this assumes that the Flutter Web app is
/// initializing "automatically", as was normal before this feature was
/// introduced. This will immediately run the `initializeEngine` and `runApp`
/// methods (via [AppBootstrap.autoStart]).
///
/// This method should NOT trigger the download of any additional resources
/// (except when the app is in "autoStart" mode).
Future<void> warmupEngine({
  Function? registerPlugins,
  Function? runApp,
}) async {
  // Create the object that knows how to bootstrap an app from JS and Dart.
  final AppBootstrap bootstrap = AppBootstrap(
    initializeEngine: ([JsFlutterConfiguration? configuration]) async {
      await initializeEngineServices(jsConfiguration: configuration);
    }, runApp: () async {
      if (registerPlugins != null) {
        registerPlugins();
      }
      await initializeEngineUi();
      if (runApp != null) {
        runApp();
      }
    },
  );

  final FlutterLoader? loader = flutter?.loader;
  if (loader == null || loader.isAutoStart) {
    // The user does not want control of the app, bootstrap immediately.
    domWindow.console.debug('Flutter Web Bootstrap: Auto.');
    await bootstrap.autoStart();
  } else {
    // Yield control of the bootstrap procedure to the user.
    domWindow.console.debug('Flutter Web Bootstrap: Programmatic.');
    loader.didCreateEngineInitializer(bootstrap.prepareEngineInitializer());
  }
}
