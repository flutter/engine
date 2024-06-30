// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' hide TextStyle;

import '../common/matchers.dart';
import '../common/test_initialization.dart';
import 'screenshot.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  const screenWidth = 600.0;
  const screenHeight = 800.0;
  const screenRect = Rect.fromLTWH(0, 0, screenWidth, screenHeight);
  const black12Color = Color(0x1F000000);
  const redAccentColor = Color(0xFFFF1744);
  const kDashLength = 5.0;

  setUpUnitTests(
    setUpTestViewDimensions: false,
  );

  test('Should calculate tangent on line', () async {
    final path = Path();
    path.moveTo(50, 130);
    path.lineTo(150, 20);

    final metric = path.computeMetrics().first;
    final t = metric.getTangentForOffset(50.0)!;
    expect(t.position.dx, within(from: 83.633, distance: 0.01));
    expect(t.position.dy, within(from: 93.0, distance: 0.01));
    expect(t.vector.dx, within(from: 0.672, distance: 0.01));
    expect(t.vector.dy, within(from: -0.739, distance: 0.01));
  });

  test('Should calculate tangent on cubic curve', () async {
    final path = Path();
    const p1x = 240.0;
    const p1y = 120.0;
    const p2x = 320.0;
    const p2y = 25.0;
    path.moveTo(150, 20);
    path.quadraticBezierTo(p1x, p1y, p2x, p2y);
    final metric = path.computeMetrics().first;
    final t = metric.getTangentForOffset(50.0)!;
    expect(t.position.dx, within(from: 187.25, distance: 0.01));
    expect(t.position.dy, within(from: 53.33, distance: 0.01));
    expect(t.vector.dx, within(from: 0.82, distance: 0.01));
    expect(t.vector.dy, within(from: 0.56, distance: 0.01));
  });

  test('Should calculate tangent on quadratic curve', () async {
    final path = Path();
    const p0x = 150.0;
    const p0y = 20.0;
    const p1x = 320.0;
    const p1y = 25.0;
    path.moveTo(150, 20);
    path.quadraticBezierTo(p0x, p0y, p1x, p1y);
    final metric = path.computeMetrics().first;
    final t = metric.getTangentForOffset(50.0)!;
    expect(t.position.dx, within(from: 199.82, distance: 0.01));
    expect(t.position.dy, within(from: 21.46, distance: 0.01));
    expect(t.vector.dx, within(from: 0.99, distance: 0.01));
    expect(t.vector.dy, within(from: 0.02, distance: 0.01));
  });

  // Test for extractPath to draw 5 pixel length dashed line using quad curve.
  test('Should draw dashed line on quadratic curve.', () async {
    final rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 500, 500));

    final paint = SurfacePaint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 3
      ..color = black12Color;
    final redPaint = SurfacePaint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1
      ..color = redAccentColor;

    final path = SurfacePath();
    path.moveTo(50, 130);
    path.lineTo(150, 20);
    const p1x = 240.0;
    const p1y = 120.0;
    const p2x = 320.0;
    const p2y = 25.0;
    path.quadraticBezierTo(p1x, p1y, p2x, p2y);

    rc.drawPath(path, paint);

    const t0 = 0.2;
    const t1 = 0.7;

    final metrics = path.computeMetrics().toList();
    var totalLength = 0.0;
    for (final m in metrics) {
      totalLength += m.length;
    }
    final dashedPath = Path();
    for (final measurePath in path.computeMetrics()) {
      var distance = totalLength * t0;
      var draw = true;
      while (distance < measurePath.length * t1) {
        const length = kDashLength;
        if (draw) {
          dashedPath.addPath(
              measurePath.extractPath(distance, distance + length),
              Offset.zero);
        }
        distance += length;
        draw = !draw;
      }
    }
    rc.drawPath(dashedPath, redPaint);
    await canvasScreenshot(rc, 'path_dash_quadratic', canvasRect: screenRect);
  });

  // Test for extractPath to draw 5 pixel length dashed line using cubic curve.
  test('Should draw dashed line on cubic curve.', () async {
    final rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 500, 500));

    final paint = SurfacePaint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 3
      ..color = black12Color;
    final redPaint = SurfacePaint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1
      ..color = redAccentColor;

    final path = Path();
    path.moveTo(50, 130);
    path.lineTo(150, 20);
    const p1x = 40.0;
    const p1y = 120.0;
    const p2x = 300.0;
    const p2y = 130.0;
    const p3x = 320.0;
    const p3y = 25.0;
    path.cubicTo(p1x, p1y, p2x, p2y, p3x, p3y);

    rc.drawPath(path, paint);

    const t0 = 0.2;
    const t1 = 0.7;

    final metrics = path.computeMetrics().toList();
    var totalLength = 0.0;
    for (final m in metrics) {
      totalLength += m.length;
    }
    final dashedPath = Path();
    for (final measurePath in path.computeMetrics()) {
      var distance = totalLength * t0;
      var draw = true;
      while (distance < measurePath.length * t1) {
        const length = kDashLength;
        if (draw) {
          dashedPath.addPath(
              measurePath.extractPath(distance, distance + length),
              Offset.zero);
        }
        distance += length;
        draw = !draw;
      }
    }
    rc.drawPath(dashedPath, redPaint);
    await canvasScreenshot(rc, 'path_dash_cubic', canvasRect: screenRect);
  });
}
