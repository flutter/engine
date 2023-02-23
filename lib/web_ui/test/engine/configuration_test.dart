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
      final FlutterConfiguration config = FlutterConfiguration.legacy(null);

      expect(config.canvasKitMaximumSurfaces, 8); // _defaultCanvasKitMaximumSurfaces
    });

    test('legacy constructor initializes with a Js Object', () async {
      final FlutterConfiguration config = FlutterConfiguration.legacy(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 16);
    });
  });

  group('setUserConfiguration', () {
    test('throws assertion error if already initialized from JS', () async {
      final FlutterConfiguration config = FlutterConfiguration.legacy(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 12,
        }) as JsFlutterConfiguration);

      expect(() {
        config.setUserConfiguration(
          js_util.jsify(<String, Object?>{
            'canvasKitMaximumSurfaces': 16,
          }) as JsFlutterConfiguration);
      }, throwsAssertionError);
    });

    test('stores config if JS configuration was null', () async {
      final FlutterConfiguration config = FlutterConfiguration.legacy(null);

      config.setUserConfiguration(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 16);
    });
  });

  group('CanvasKit config', () {
    test('validates canvasKitJsFileName', () {
      final FlutterConfiguration config = FlutterConfiguration();

      config.setUserConfiguration(
        js_util.jsify(<String, Object?>{'canvasKitJsFileName': 'foo'}) as JsFlutterConfiguration,
      );
      expect(() => config.canvasKitJsFileNames, throwsArgumentError);

      config.setUserConfiguration(
        js_util.jsify(<String, Object?>{'canvasKitJsFileName': 'foo.js'}) as JsFlutterConfiguration,
      );
      expect(config.canvasKitJsFileNames, <String>['foo.js']);
    });

    test('default canvasKitJsFileNames', () {
      final FlutterConfiguration config = FlutterConfiguration();

      expect(
        config.canvasKitJsFileNames,
        hasLength(allOf(greaterThan(0), lessThanOrEqualTo(2))),
      );
      expect(config.canvasKitJsFileNames.last, 'canvaskit.js');
    });
  });
}
