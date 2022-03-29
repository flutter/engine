// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:js' as js;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart' as engine;
import 'package:ui/ui.dart' as ui;

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  test('webOnlyWarmupEngine calls _flutter.loader.didLoadMainDartJs callback', () async {
    late js.JsObject engineInitializer;

    void registerPluginsMock() {}
    void runAppMock() {}
    void didLoadMainDartJsMock (js.JsObject obj) {
      engineInitializer = obj;
    }

    // Prepare the DOM for: _flutter.loader.didLoadMainDartJs
    js.context['_flutter'] = js.JsObject.jsify(<String, Object>{
      'loader': js.JsObject.jsify(<String, Object>{
        'didLoadMainDartJs': js.allowInterop(didLoadMainDartJsMock)
      })
    });

    // Reset the engine
    engine.debugResetEngineInitializationState();

    await ui.webOnlyWarmupEngine(
      registerPlugins: registerPluginsMock,
      runApp: runAppMock,
    );

    // Check that the object we captured is actually a loader
    expect(engineInitializer, isNotNull);
    expect(engineInitializer.hasProperty('initializeEngine'), isTrue, reason: 'Missing FlutterEngineInitializer method: initializeEngine.');
    expect(engineInitializer.hasProperty('runApp'), isTrue, reason: 'Missing FlutterEngineInitializer method: runApp.');

    js.context['_flutter'] = null;
  });

  test('webOnlyWarmupEngine does auto-start when _flutter.loader.didLoadMainDartJs does not exist', () async {
    bool pluginsRegistered = false;
    bool appRan = false;
    void registerPluginsMock() {
      pluginsRegistered = true;
    }
    void runAppMock() {
      appRan = true;
    }

    // Reset the engine
    engine.debugResetEngineInitializationState();

    await ui.webOnlyWarmupEngine(
      registerPlugins: registerPluginsMock,
      runApp: runAppMock,
    );

    // Check that the object we captured is actually a loader
    expect(pluginsRegistered, isTrue, reason: 'Plugins should be immediately registered in autoStart mode.');
    expect(appRan, isTrue, reason: 'App should run immediately in autoStart mode');
  });
  // We cannot test anymore, because by now the engine has registered some stuff that can't be rewound back.
  // Like the `ext.flutter.disassemble` developer extension.
}
