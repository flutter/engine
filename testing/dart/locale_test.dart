// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:test/test.dart';

void main() {
  group('Locale:', () {
    test('Unnamed constructor', () {
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

    group('Standards compliance, BCP 47 and LDML:', () {
      test('Safe constructor.', () {
        expect(Locale.safe('IW').languageCode, 'he',
            reason: "Case insensitive to input.");
        expect(Locale.safe('root').languageCode, 'und',
            reason: "BCP 47 Language Tag Conversion: "
                "replace 'root' with 'und'.");
        expect(Locale.safe('ROOT').languageCode, 'und',
            reason: "BCP 47 Language Tag Conversion: "
                "replace 'root' with 'und', and case insensitive.");

        expect(Locale.safe('a').languageCode, 'und',
            reason: "One character language subtags are invalid.");
        expect(Locale.safe('abcd').languageCode, 'und',
            reason: "The only valid 4 character language subtag is 'root'.");
        expect(Locale.safe('abcdefghi').languageCode, 'und',
            reason: "Language subtags may not be more than 8 characters.");

        expect(Locale.safe('IW').languageCode, 'und',
            reason: "One character language subtags are invalid.");
        expect(Locale.safe('en').languageCode, 'und',
            reason: "The only valid 4 character language subtag is 'root'.");
        expect(Locale.safe('en').languageCode, 'und',
            reason: "Language subtags may not be more than 8 characters.");
      });
    });
  });
}
