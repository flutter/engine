// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// Bootstraps the Flutter Web engine and app.
///
/// If the app uses plugins, then the [registerPlugins] callback can be provided
/// to register those plugins. This is done typically by calling
/// `registerPlugins` from the auto-generated `web_plugin_registrant.dart` file.
///
/// The [runApp] callback is invoked to run the app after the engine is fully
/// initialized.
///
/// For more information, see what the `flutter_tools` does in the entrypoint
/// that it generates around the app's main method:
///
/// * https://github.com/flutter/flutter/blob/95be76ab7e3dca2def54454313e97f94f4ac4582/packages/flutter_tools/lib/src/web/file_generators/main_dart.dart#L14-L43
///
/// By default, engine initialization and app startup occur immediately and back
/// to back. They can be programmatically controlled by setting
/// `FlutterLoader.didCreateEngineInitializer`. For more information, see how
/// `flutter.js` does it:
///
/// * https://github.com/flutter/flutter/blob/95be76ab7e3dca2def54454313e97f94f4ac4582/packages/flutter_tools/lib/src/web/file_generators/js/flutter.js
Future<void> bootstrapEngine({
  ui.VoidCallback? registerPlugins,
  ui.VoidCallback? runApp,
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

/// Loads manifest fonts.
///
/// By default manifest fonts are loaded automatically as part of engine
/// initialization. However, that may delay the invocation of the app's `main`
/// function. If the app needs to start doing useful work before the fonts are
/// fully loaded, such as render UI without text (e.g. a loading indicator), or
/// perform non-UI work (e.g. make early HTTP calls), then the configuration
/// option `loadManifestFontsBeforeAppMain` can be set to `false`. This will
/// make the engine skip font loading. Instead, the app can call this function
/// directly when appropriate.
///
/// Beware that attempting to render text before fonts are loaded may lead to
/// improper text rendering. It is expected that the developer has enough
/// control over the app's code to ensure the correct loading sequence.
Future<void> loadManifestFonts() async {
  await downloadAssetFonts();
  sendFontChangeMessage();
}
