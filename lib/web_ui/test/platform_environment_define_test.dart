// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  test('Web library environment define exists', () {
    expect(const bool.fromEnvironment('dart.library.html'), isTrue);
    expect(const bool.fromEnvironment('dart.library.someFooLibrary'), isFalse);
  });
}
