// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/platform_dispatcher/locale_utils.dart';
import 'package:ui/ui.dart' as ui;

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  ensureFlutterViewEmbedderInitialized();

  group('parseLanguages', () {
    test('converts language strings into Locale objects', () async {
      final List<ui.Locale> locales = parseLanguages(
        <String>['en-US', 'en-GB-oxendict', 'zh', 'ast'],
      );

      expect(locales, <ui.Locale>[
        const ui.Locale('en', 'US'),
        const ui.Locale.fromSubtags(
          languageCode: 'en',
          countryCode: 'GB',
          scriptCode: 'oxendict',
        ),
        const ui.Locale('zh'),
        const ui.Locale.fromSubtags(
          languageCode: 'ast',
        ),
      ]);
    });

    test('computes scriptCode for zh locales with country code', () async {
      final ui.Locale zhLocale = parseLanguages(
        <String>['zh'],
      ).first;
      expect(
        zhLocale.scriptCode,
        isNull,
        reason: 'zh language should be left untouched.',
      );

      final List<ui.Locale> hantLocales = parseLanguages(
        <String>['zh-HK', 'zh-MO', 'zh-TW', 'zh-Hant-SG'],
      );
      expect(
          hantLocales,
          everyElement(predicate(
            (ui.Locale locale) => locale.scriptCode == 'Hant',
            'scriptCode should be "Hant"',
          )));

      final List<ui.Locale> hansLocales = parseLanguages(
        <String>['zh-CN', 'zh-SG', 'zh-Hans-TW'],
      );
      expect(
          hansLocales,
          everyElement(predicate(
            (ui.Locale locale) => locale.scriptCode == 'Hans',
            'scriptCode should be "Hans"',
          )));
    });
  });
}
