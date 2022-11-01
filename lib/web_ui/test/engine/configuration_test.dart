// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@TestOn('browser')

import 'package:js/js_util.dart' as js_util;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('FlutterConfiguration', () {
    test('initializes with null', () async {
      final FlutterConfiguration config = FlutterConfiguration(null);

      expect(config.canvasKitMaximumSurfaces, 8); // _defaultCanvasKitMaximumSurfaces
    });

    test('initializes with a Js Object', () async {
      final FlutterConfiguration config = FlutterConfiguration(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 16);
    });
  });

  group('addConfigurationOverrides', () {
    test('accepts a null override', () async {
      final FlutterConfiguration config = FlutterConfiguration(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      config.addConfigurationOverrides(null);

      expect(config.canvasKitMaximumSurfaces, 16);
    });

    test('accepts a Js Object override', () async {
      final FlutterConfiguration config = FlutterConfiguration(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      config.addConfigurationOverrides(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 8,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 8);
    });

    test('returns last non-null value set for a given field', () async {
      final FlutterConfiguration config = FlutterConfiguration(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 16,
        }) as JsFlutterConfiguration);

      config.addConfigurationOverrides(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': null,
        }) as JsFlutterConfiguration);

      config.addConfigurationOverrides(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': 8,
        }) as JsFlutterConfiguration);

      config.addConfigurationOverrides(
        js_util.jsify(<String, Object?>{
          'canvasKitMaximumSurfaces': null,
        }) as JsFlutterConfiguration);

      expect(config.canvasKitMaximumSurfaces, 8);
    });
  });

  group('getConfigValue', () {

    test('Without configs, the extractor does not run (default returned)', () async {
      final FlutterConfiguration config = FlutterConfiguration(null);

      final String value = config.getConfigValue<String>(
        (JsFlutterConfiguration c) => throw Exception('Fail'), 'Pass');

      expect(value, 'Pass');
    });

    test('Returns anything that the extractor finds not null', () async {
      final FlutterConfiguration config = FlutterConfiguration(
        js_util.jsify(<String, Object?>{}) as JsFlutterConfiguration);

      final String value = config.getConfigValue<String>(
        (JsFlutterConfiguration c) => 'Pass', 'Fail');

      expect(value, 'Pass');
    });

    test('Returns the default if the extractor finds only null', () async {
      final FlutterConfiguration config = FlutterConfiguration(null);

      final String value = config.getConfigValue<String>(
        (JsFlutterConfiguration c) => null, 'Default');

      expect(value, 'Default');
    });
  });
}
