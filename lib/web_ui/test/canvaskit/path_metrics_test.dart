// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'package:test/test.dart';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

void main() {
  setUpAll(() async {
    await ui.webOnlyInitializePlatform();
  });

  test('Using CanvasKit', () {
    expect(experimentalUseSkia, true);
  });

  test(SkPathMetrics, () {
    final ui.Path path = ui.Path();
    expect(path, isA<SkPath>());
    expect(path.computeMetrics().length, 0);

    path.addRect(ui.Rect.fromLTRB(0, 0, 10, 10));
    final ui.PathMetric metric = path.computeMetrics().single;
    expect(metric.contourIndex, 0);
    expect(metric.extractPath(0, 0.5).computeMetrics().length, 1);

    final ui.Tangent tangent1 = metric.getTangentForOffset(5);
    expect(tangent1.position, ui.Offset(5, 0));
    expect(tangent1.vector, ui.Offset(1, 0));

    final ui.Tangent tangent2 = metric.getTangentForOffset(15);
    expect(tangent2.position, ui.Offset(10, 5));
    expect(tangent2.vector, ui.Offset(0, 1));

    expect(metric.isClosed, true);

    path.addOval(ui.Rect.fromLTRB(10, 10, 100, 100));
    expect(path.computeMetrics().length, 2);
  });
}
