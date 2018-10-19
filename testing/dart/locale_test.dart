// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:test/test.dart';

void main() {
  test('Locale', () {
    final Null $null = null;
    expect(const Locale('en').toString(), 'en');
    expect(const Locale('en'), new Locale('en', $null));
    expect(const Locale('en').hashCode, new Locale('en', $null).hashCode);
    expect(const Locale('en'), isNot(new Locale('en', '')));
    expect(const Locale('en').hashCode, isNot(new Locale('en', '').hashCode));
    expect(const Locale('en', 'US').toString(), 'en_US');
    expect(const Locale('iw').toString(), 'he');
    expect(const Locale('iw', 'DD').toString(), 'he_DE');
    expect(const Locale('iw', 'DD'), const Locale('he', 'DE'));
  });

  test('Locale.fromSubtags', () {
    expect(const Locale.fromSubtags().languageCode, 'und');
    expect(const Locale.fromSubtags().scriptCode, null);
    expect(const Locale.fromSubtags().countryCode, null);
    expect(const Locale.fromSubtags().variants, orderedEquals([]));

    expect(const Locale.fromSubtags(language: 'en').toString(), 'en');
    expect(const Locale.fromSubtags(language: 'en').languageCode, 'en');
    expect(const Locale.fromSubtags(script: 'Latn').toString(), 'und_Latn');
    expect(const Locale.fromSubtags(script: 'Latn').scriptCode, 'Latn');
    expect(const Locale.fromSubtags(region: 'US').toString(), 'und_US');
    expect(const Locale.fromSubtags(region: 'US').countryCode, 'US');
    expect(const Locale.fromSubtags(variants: []).toString(), 'und');
    expect(const Locale.fromSubtags(variants: []).variants,
           orderedEquals([]));
    expect(const Locale.fromSubtags(variants: ['fonipa', 'scouse']).variants,
           orderedEquals(['fonipa', 'scouse']));

    expect(Locale.fromSubtags(language: 'es', region: '419').toString(), 'es_419');
    expect(Locale.fromSubtags(language: 'es', region: '419').languageCode, 'es');
    expect(Locale.fromSubtags(language: 'es', region: '419').countryCode, '419');
    expect(Locale.fromSubtags(script: 'Latn', variants: ['fonipa']).toString(), 'und_Latn_fonipa');
    expect(Locale.fromSubtags(script: 'Latn', variants: ['fonipa']).scriptCode, 'Latn');
    expect(Locale.fromSubtags(script: 'Latn', variants: ['fonipa']).variants,
           orderedEquals(['fonipa']));

    expect(Locale.fromSubtags(language: 'zh', script: 'Hans', region: 'CN').toString(), 'zh_Hans_CN');
  });

  test('Locale equality', () {
    expect(Locale.fromSubtags(language: 'en', variants: ['fonipa']),
           Locale.fromSubtags(language: 'en', variants: ['fonipa']));
    expect(Locale.fromSubtags(language: 'en', variants: ['fonipa']).hashCode,
           Locale.fromSubtags(language: 'en', variants: ['fonipa']).hashCode);
    expect(Locale.fromSubtags(language: 'en'),
           isNot(Locale.fromSubtags(language: 'en', script: 'Latn')));
    expect(Locale.fromSubtags(language: 'en').hashCode,
           isNot(Locale.fromSubtags(language: 'en', script: 'Latn').hashCode));
  });
}
