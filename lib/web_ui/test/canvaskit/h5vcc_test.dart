// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('H5vcc patched CanvasKit', () {
    int getH5vccSkSurfaceCalledCount = 0;

    patchH5vccCanvasKit(onGetH5vccSkSurfaceCalled: () {
      getH5vccSkSurfaceCalledCount++;
    });
    setUpCanvasKitTest();

    setUp(() {
      getH5vccSkSurfaceCalledCount = 0;
    });

    test('sets useH5vccCanvasKit', () {
      expect(useH5vccCanvasKit, true);
    });

    test('API includes patched getH5vccSkSurface', () {
      expect(canvasKit.getH5vccSkSurface, isNotNull);
    });

    test('Surface scquireFrame uses getH5vccSkSurface', () {
      final Surface surface = SurfaceFactory.instance.getSurface();
      surface.acquireFrame(ui.Size.zero);
      expect(getH5vccSkSurfaceCalledCount, 1);
    });
  }, testOn: 'chrome');
}
