// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' hide TextStyle;

import '../screenshot.dart';
import '../testimage.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

SurfacePaint makePaint() => Paint() as SurfacePaint;

Future<void> testMain() async {
  const screenWidth = 100.0;
  const screenHeight = 100.0;
  const region = Rect.fromLTWH(0, 0, screenWidth, screenHeight);

  // Regression test for https://github.com/flutter/flutter/issues/76966
  test('Draws image with dstATop color filter', () async {
    final canvas = RecordingCanvas(region);
    canvas.drawImage(
        createFlutterLogoTestImage(),
        const Offset(10, 10),
        makePaint()
          ..colorFilter =
              const EngineColorFilter.mode(Color(0x40000000), BlendMode.dstATop));
    await canvasScreenshot(canvas, 'image_color_fiter_dstatop', region: region);
  });

  test('Draws image with matrix color filter', () async {
    final canvas = RecordingCanvas(region);
    canvas.drawImage(
        createFlutterLogoTestImage(),
        const Offset(10, 10),
        makePaint()
          ..colorFilter = const EngineColorFilter.matrix(<double>[
            0.2126, 0.7152, 0.0722, 0, 0, //
            0.2126, 0.7152, 0.0722, 0, 0, //
            0.2126, 0.7152, 0.0722, 0, 0, //
            0, 0, 0, 1, 0, //
          ]));
    await canvasScreenshot(canvas, 'image_matrix_color_fiter', region: region);
  });
}
