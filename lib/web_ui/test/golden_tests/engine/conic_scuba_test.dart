// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import 'package:flutter_web_ui/src/engine.dart';
import 'package:flutter_web_ui/ui.dart';
import 'package:test/test.dart';

import 'scuba.dart';

void main() async {
  final EngineScubaTester scuba = await EngineScubaTester.initialize(
    viewportSize: const Size(600, 800),
  );

  Future<void> testPath(Path path, String scubaFileName) async {
    const Rect canvasBounds = Rect.fromLTWH(0, 0, 600, 800);
    final BitmapCanvas bitmapCanvas = BitmapCanvas(canvasBounds);
    final RecordingCanvas canvas = RecordingCanvas(canvasBounds);

    Paint paint = Paint()
      ..color = const Color(0x7F7F7F7F)
      ..style = PaintingStyle.fill;

    canvas.drawPath(path, paint);

    paint = Paint()
      ..strokeWidth = 2.0
      ..color = const Color(0xFF7F007F)
      ..style = PaintingStyle.stroke;

    canvas.drawPath(path, paint);

    html.document.body.append(bitmapCanvas.rootElement);
    canvas.apply(bitmapCanvas);
    await scuba.diffScreenshot(scubaFileName);
    bitmapCanvas.rootElement.remove();
  }

  test('render conic with control point horizontal center', () async {
    const double yStart = 20;

    const Offset p0 = Offset(25, yStart + 25);
    const Offset pc = Offset(60, yStart + 150);
    const Offset p2 = Offset(100, yStart + 50);

    final Path path = Path();
    path.moveTo(p0.dx, p0.dy);
    path.conicTo(pc.dx, pc.dy, p2.dx, p2.dy, 0.5);
    path.close();
    path.moveTo(p0.dx, p0.dy + 200);
    path.conicTo(pc.dx, pc.dy + 200, p2.dx, p2.dy + 200, 10);
    path.close();

    await testPath(path, 'render_conic_1_w10');
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('render conic with control point left of start point', () async {
    const double yStart = 20;

    const Offset p0 = Offset(60, yStart + 25);
    const Offset pc = Offset(25, yStart + 150);
    const Offset p2 = Offset(100, yStart + 50);

    final Path path = Path();
    path.moveTo(p0.dx, p0.dy);
    path.conicTo(pc.dx, pc.dy, p2.dx, p2.dy, 0.5);
    path.close();
    path.moveTo(p0.dx, p0.dy + 200);
    path.conicTo(pc.dx, pc.dy + 200, p2.dx, p2.dy + 200, 10);
    path.close();

    await testPath(path, 'render_conic_2_w10');
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('render conic with control point above start point', () async {
    const double yStart = 20;

    const Offset p0 = Offset(25, yStart + 125);
    const Offset pc = Offset(60, yStart + 50);
    const Offset p2 = Offset(100, yStart + 150);

    final Path path = Path();
    path.moveTo(p0.dx, p0.dy);
    path.conicTo(pc.dx, pc.dy, p2.dx, p2.dy, 0.5);
    path.close();
    path.moveTo(p0.dx, p0.dy + 200);
    path.conicTo(pc.dx, pc.dy + 200, p2.dx, p2.dy + 200, 10);
    path.close();

    await testPath(path, 'render_conic_2');
  }, timeout: const Timeout(Duration(seconds: 10)));
}
