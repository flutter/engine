// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@TestOn('browser')

import 'package:js/js_util.dart' as js_util;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import '../matchers.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('FlutterConfiguration', () {
    test('initializes with null', () async {
      final FlutterConfiguration config = FlutterConfiguration.fromJsGlobals(null);

      expect(config.canvasKitMaximumSurfaces, 8); // _defaultCanvasKitMaximumSurfaces
    });

    test('fromJsGlobals initializes with a Js Object', () async {
      final FlutterConfiguration config = FlutterConfiguration.fromJsGlobals(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 16);
    });
  });

  group('setRuntimeConfiguration', () {
    test('throws assertion error if already initialized from JS', () async {
      final FlutterConfiguration config = FlutterConfiguration.fromJsGlobals(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 12,
        }) as JsFlutterConfiguration);

      expect(() {
        config.setRuntimeConfiguration(
          js_util.jsify(<String, Object?>{
            'canvasKitMaximumSurfaces': 16,
          }) as JsFlutterConfiguration);
      }, throwsAssertionError);
    });

    test('stores config if JS configuration was null', () async {
      final FlutterConfiguration config = FlutterConfiguration.fromJsGlobals(null);

      config.setRuntimeConfiguration(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 16);
    });
  });
}
