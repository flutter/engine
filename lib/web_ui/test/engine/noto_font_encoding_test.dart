// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine/noto_font.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  void roundTrip(List<CodePointRange> ranges) {
    String rangesToString(List ranges) => ranges
        .map((CodePointRange range) => '(${range.start}, ${range.end})')
        .join(',');

    final String encoding = packFontRanges(ranges);
    final NotoFont font = NotoFont('Test Sans', 'uri', encoding);
    final List<CodePointRange> computedRanges = font.computeUnicodeRanges();
    expect(rangesToString(computedRanges), rangesToString(ranges));
  }

  test('Simple', () {
    // 'Noto Sans Deseret' v15
    roundTrip([
      CodePointRange(0x20, 0x20),
      CodePointRange(0xa0, 0xa0),
      CodePointRange(0x10400, 0x1044f),
    ]);
  });

  test('Can encode from zero', () {
    roundTrip([CodePointRange(0, 0)]);
    roundTrip([CodePointRange(0, 23), CodePointRange(32, 32)]);
  });
}
