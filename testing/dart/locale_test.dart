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
    expect(const Locale('en', 'US').toString(), 'en-US');
    expect(const Locale('iw').toString(), 'he');
    expect(const Locale('iw', 'DD').toString(), 'he-DE');
    expect(const Locale('iw', 'DD'), const Locale('he', 'DE'));
  });

  test('Locale.fromComponents', () {
    expect(const Locale.fromComponents().languageCode, 'und');
    expect(const Locale.fromComponents().scriptCode, null);
    expect(const Locale.fromComponents().countryCode, null);
    expect(const Locale.fromComponents().variants, orderedEquals([]));

    expect(const Locale.fromComponents(language: 'en').toString(), 'en');
    expect(const Locale.fromComponents(language: 'en').languageCode, 'en');
    expect(const Locale.fromComponents(script: 'Latn').toString(), 'und-Latn');
    expect(const Locale.fromComponents(script: 'Latn').scriptCode, 'Latn');
    expect(const Locale.fromComponents(region: 'US').toString(), 'und-US');
    expect(const Locale.fromComponents(region: 'US').countryCode, 'US');
    expect(const Locale.fromComponents(variants: 'fonipa-scouse').variants,
           orderedEquals(['fonipa', 'scouse']));

    expect(Locale.fromComponents(language: 'es', region: '419').toString(), 'es-419');
    expect(Locale.fromComponents(language: 'es', region: '419').languageCode, 'es');
    expect(Locale.fromComponents(language: 'es', region: '419').countryCode, '419');
    expect(Locale.fromComponents(script: 'Latn', variants: 'fonipa').toString(), 'und-Latn-fonipa');
    expect(Locale.fromComponents(script: 'Latn', variants: 'fonipa').scriptCode, 'Latn');
    expect(Locale.fromComponents(script: 'Latn', variants: 'fonipa').variants,
           orderedEquals(['fonipa']));

    expect(Locale.fromComponents(language: 'zh', script: 'Hans', region: 'CN').toString(), 'zh-Hans-CN');
  });
}
