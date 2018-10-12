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

  test('Locale.fromComponents', () {
    expect(
      const Locale.fromComponents(language: 'en').toString(),
      'en',
    );
    expect(
      const Locale.fromComponents(script: 'Latn').toString(),
      'und_Latn',
    );
    expect(
      const Locale.fromComponents(region: 'US').toString(),
      'und_US',
    );
    expect(
      const Locale.fromComponents(variants: 'fonipa').toString(),
      'und_fonipa',
    );
    expect(
      const Locale.fromComponents(language: 'zh', script: 'Hans', region: 'CN')
          .toString(),
      'zh_Hans_CN',
    );
  });
}
