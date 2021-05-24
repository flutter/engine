// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import '../matchers.dart';
import 'common.dart';

const MethodCodec codec = StandardMethodCodec();

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$SurfaceFactory', () {
    setUpCanvasKitTest();

    setUp(() {
      window.debugOverrideDevicePixelRatio(1);
    });

    test('cannot be created with size less than 2', () {
      expect(() => SurfaceFactory(-1), throwsAssertionError);
      expect(() => SurfaceFactory(0), throwsAssertionError);
      expect(() => SurfaceFactory(1), throwsAssertionError);
      expect(SurfaceFactory(2), isNotNull);
    });

    // TODO: https://github.com/flutter/flutter/issues/60040
  }, skip: isIosSafari);
}
