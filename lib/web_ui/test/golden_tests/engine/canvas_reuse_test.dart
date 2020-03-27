// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:html' as html;
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/ui.dart' hide TextStyle;
import 'package:ui/src/engine.dart';
import 'package:test/test.dart';

import '../../matchers.dart';
import 'package:web_engine_tester/golden_tester.dart';

void main() async {
  const double screenWidth = 600.0;
  const double screenHeight = 800.0;
  const Rect screenRect = Rect.fromLTWH(0, 0, screenWidth, screenHeight);
  final Paint testPaint = Paint()
    ..style = PaintingStyle.stroke
    ..strokeWidth = 2.0
    ..color = const Color(0xFFFF00FF);

  setUp(() async {
    debugEmulateFlutterTesterEnvironment = true;
    await webOnlyInitializePlatform();
    webOnlyFontCollection.debugRegisterTestFonts();
    await webOnlyFontCollection.ensureFontsLoaded();
  });

  test('Reuses canvas across', () async {
    final EngineCanvas engineCanvas = BitmapCanvas(screenRect);
    const Rect region = Rect.fromLTWH(0, 0, 500, 500);

    // Draw first frame into engine canvas.
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTWH(1, 2, 300, 400));
    final Path path = Path()
      ..moveTo(3, 0)
      ..lineTo(100, 97);
    rc.drawPath(path, testPaint);
    rc.apply(engineCanvas);
    engineCanvas.endOfPaint();

    html.Element sceneElement = html.Element.tag('flt-scene');
    sceneElement.append(engineCanvas.rootElement);
    html.document.body.append(sceneElement);

    final html.CanvasElement canvas = html.document.querySelector('canvas');
    expect(canvas , isNotNull);

    // Add id to canvas element to test for reuse.
    const String kTestId = 'test-id-5698';
    canvas.id = kTestId;

    sceneElement.remove();
    // Clear so resources are marked for reuse.

    engineCanvas.clear();

    // Now paint a second scene to same [BitmapCanvas].
    final RecordingCanvas rc2 =
        RecordingCanvas(const Rect.fromLTWH(1, 2, 300, 400));
    final Path path2 = Path()
      ..moveTo(3, 0)
      ..quadraticBezierTo(100, 0, 100, 100);
    rc2.drawPath(path2, testPaint);
    rc2.apply(engineCanvas);

    sceneElement = html.Element.tag('flt-scene');
    sceneElement.append(engineCanvas.rootElement);
    html.document.body.append(sceneElement);

    final html.CanvasElement canvas2 = html.document.querySelector('canvas');
    expect(canvas2.id, kTestId);
    await matchGoldenFile('bitmap_canvas_reuse1.png', region: region, write: true);
  });
}

const String _base64Encoded20x20TestImage = 'iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAIAAAAC64paAAAACXBIWXMAAC4jAAAuIwF4pT92AAAA'
    'B3RJTUUH5AMFFBksg4i3gQAAABl0RVh0Q29tbWVudABDcmVhdGVkIHdpdGggR0lNUFeBDhcAAAAj'
    'SURBVDjLY2TAC/7jlWVioACMah4ZmhnxpyHG0QAb1UyZZgBjWAIm/clP0AAAAABJRU5ErkJggg==';

HtmlImage _createRealTestImage() {
  return HtmlImage(
    html.ImageElement()
      ..src = 'data:text/plain;base64,$_base64Encoded20x20TestImage',
    20,
    20,
  );
}

class TestImage implements Image {
  @override
  int get width => 20;

  @override
  int get height => 10;

  @override
  Future<ByteData> toByteData(
      {ImageByteFormat format = ImageByteFormat.rawRgba}) async {
    throw UnsupportedError('Cannot encode test image');
  }

  @override
  String toString() => '[$width\u00D7$height]';

  @override
  void dispose() {}
}
