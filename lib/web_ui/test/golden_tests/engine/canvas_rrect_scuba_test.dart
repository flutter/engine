// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import 'package:flutter_web_ui/src/engine.dart';
import 'package:flutter_web_ui/ui.dart';
import 'package:test/test.dart';

import 'scuba.dart';

void main() async {
  // Scuba doesn't give us viewport smaller than 472px wide.
  final EngineScubaTester scuba = await EngineScubaTester.initialize(
    viewportSize: const Size(500, 100),
  );

  BitmapCanvas canvas;

  final PaintData niceRRectPaint = PaintData()
    ..color = const Color.fromRGBO(250, 186, 218, 1.0) // #fabada
    ..style = PaintingStyle.fill;

  // Some values to see how the algo behaves as radius get absurdly large
  const List<double> rRectRadii = <double>[0, 10, 20, 80, 8000];

  const Radius someFixedRadius = Radius.circular(10);

  setUp(() {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 500, 100));
    canvas.translate(10, 10); // Center
  });

  tearDown(() {
    canvas.rootElement.remove();
  });

  test('round square with big (equal) radius ends up as a circle', () async {
    for (int i = 0; i < 5; i++) {
      canvas.drawRRect(
          RRect.fromRectAndRadius(Rect.fromLTWH(100 * i.toDouble(), 0, 80, 80),
              Radius.circular(rRectRadii[i])),
          niceRRectPaint);
    }

    html.document.body.append(canvas.rootElement);
    await scuba.diffScreenshot('canvas_rrect_round_square');
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('round rect with big radius scale down smaller radius', () async {
    for (int i = 0; i < 5; i++) {
      final Radius growingRadius = Radius.circular(rRectRadii[i]);
      final RRect rrect = RRect.fromRectAndCorners(
          Rect.fromLTWH(100 * i.toDouble(), 0, 80, 80),
          bottomRight: someFixedRadius,
          topRight: growingRadius,
          bottomLeft: growingRadius);

      canvas.drawRRect(rrect, niceRRectPaint);
    }

    html.document.body.append(canvas.rootElement);
    await scuba.diffScreenshot('canvas_rrect_overlapping_radius');
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('diff round rect with big radius scale down smaller radius', () async {
    for (int i = 0; i < 5; i++) {
      final Radius growingRadius = Radius.circular(rRectRadii[i]);
      final RRect outerRRect = RRect.fromRectAndCorners(
          Rect.fromLTWH(100 * i.toDouble(), 0, 80, 80),
          bottomRight: someFixedRadius,
          topRight: growingRadius,
          bottomLeft: growingRadius);

      // Inner is half of outer, but offset a little so it looks nicer
      final RRect innerRRect = RRect.fromRectAndCorners(
          Rect.fromLTWH(100 * i.toDouble() + 5, 5, 40, 40),
          bottomRight: someFixedRadius / 2,
          topRight: growingRadius / 2,
          bottomLeft: growingRadius / 2);

      canvas.drawDRRect(outerRRect, innerRRect, niceRRectPaint);
    }

    html.document.body.append(canvas.rootElement);
    await scuba.diffScreenshot('canvas_drrect_overlapping_radius');
  }, timeout: const Timeout(Duration(seconds: 10)));
}
