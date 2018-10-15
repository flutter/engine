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

  test('Locale.create', () {
    expect(const Locale.create().languageCode, 'und');
    expect(const Locale.create().scriptCode, null);
    expect(const Locale.create().countryCode, null);
    expect(const Locale.create().variants, orderedEquals([]));

    expect(const Locale.create(language: 'en').toString(), 'en');
    expect(const Locale.create(language: 'en').languageCode, 'en');
    expect(const Locale.create(script: 'Latn').toString(), 'und_Latn');
    expect(const Locale.create(script: 'Latn').scriptCode, 'Latn');
    expect(const Locale.create(region: 'US').toString(), 'und_US');
    expect(const Locale.create(region: 'US').countryCode, 'US');
    expect(const Locale.create(variants: []).variants,
           orderedEquals([]));
    expect(const Locale.create(variants: ['fonipa', 'scouse']).variants,
           orderedEquals(['fonipa', 'scouse']));

    expect(Locale.create(language: 'es', region: '419').toString(), 'es_419');
    expect(Locale.create(language: 'es', region: '419').languageCode, 'es');
    expect(Locale.create(language: 'es', region: '419').countryCode, '419');
    expect(Locale.create(script: 'Latn', variants: ['fonipa']).toString(), 'und_Latn_fonipa');
    expect(Locale.create(script: 'Latn', variants: ['fonipa']).scriptCode, 'Latn');
    expect(Locale.create(script: 'Latn', variants: ['fonipa']).variants,
           orderedEquals(['fonipa']));

    expect(Locale.create(language: 'zh', script: 'Hans', region: 'CN').toString(), 'zh_Hans_CN');
  });
}
