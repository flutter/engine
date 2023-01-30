// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  setUpCanvasKitTest();

  group('$CkIntlFragmenter("word")', () {
    test('fragments text into words', () {
      final CkIntlFragmenter fragmenter =
          CkIntlFragmenter('Hello world 你好世界', IntlSegmenterGranularity.word);
      final Uint32List breaks = fragmenter.fragment();
      expect(
        breaks,
        orderedEquals(<int>[0, 5, 6, 11, 12, 14, 16]),
      );
    });
  }, skip: !useClientICU);

  group('$CkIntlFragmenter("grapheme")', () {
    test('fragments text into grapheme clusters', () {
      // The smiley emoji has a length of 2.
      // The family emoji has a length of 11.
      final CkIntlFragmenter fragmenter = CkIntlFragmenter(
        'Hello🙂world👨‍👩‍👧‍👦',
        IntlSegmenterGranularity.grapheme,
      );
      final Uint32List breaks = fragmenter.fragment();
      expect(
        breaks,
        orderedEquals(<int>[0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 23]),
      );
    });
  }, skip: !useClientICU);
}
