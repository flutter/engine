// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/ui.dart' hide TextStyle;
import 'package:ui/src/engine.dart';
import 'screenshot.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() async {

  setUp(() async {
    debugEmulateFlutterTesterEnvironment = true;
    await webOnlyInitializePlatform();
    webOnlyFontCollection.debugRegisterTestFonts();
    await webOnlyFontCollection.ensureFontsLoaded();
  });

  Future<void> _testGradient(String fileName, Shader shader,
      {Rect paintRect = const Rect.fromLTRB(50, 50, 300, 300),
      Rect shaderRect = const Rect.fromLTRB(50, 50, 300, 300),
      bool write: false}) async {
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 500, 500));
    final Paint paint = Paint()..shader = shader;
    final Path path = Path();
    path.addRect(paintRect);
    rc.drawPath(path, paint);
    await canvasScreenshot(rc, fileName, write: write);
  }

  test('Should draw centered radial gradient.', () async {
    Rect shaderRect = const Rect.fromLTRB(50, 50, 300, 300);
    await _testGradient(
        'radial_gradient_centered',
        Gradient.radial(
            Offset((shaderRect.left + shaderRect.right) / 2,
                (shaderRect.top + shaderRect.bottom) / 2),
            shaderRect.width / 2,
            [
              const Color.fromARGB(255, 0, 0, 0),
              const Color.fromARGB(255, 0, 0, 255)
            ]),
        shaderRect: shaderRect);
  });

  test('Should draw right bottom centered radial gradient.', () async {
    Rect shaderRect = const Rect.fromLTRB(50, 50, 300, 300);
    await _testGradient(
        'radial_gradient_right_bottom',
        Gradient.radial(
            Offset(shaderRect.right, shaderRect.bottom), shaderRect.width / 2, [
          const Color.fromARGB(255, 0, 0, 0),
          const Color.fromARGB(255, 0, 0, 255)
        ]),
        shaderRect: shaderRect);
  });

  test('Should draw with radial gradient with TileMode.clamp.', () async {
    Rect shaderRect = const Rect.fromLTRB(50, 50, 100, 100);
    await _testGradient(
        'radial_gradient_tilemode_clamp',
        Gradient.radial(
            Offset((shaderRect.left + shaderRect.right) / 2,
                (shaderRect.top + shaderRect.bottom) / 2),
            shaderRect.width / 2,
            [
              const Color.fromARGB(255, 0, 0, 0),
              const Color.fromARGB(255, 0, 0, 255)
            ],
            <double>[0.0, 1.0],
            TileMode.clamp),
        shaderRect: shaderRect);
  });

  const List<Color> colors = <Color>[
    Color(0xFF000000),
    Color(0xFFFF3C38),
    Color(0xFFFF8C42),
    Color(0xFFFFF275),
    Color(0xFF6699CC),
    Color(0xFF656D78),];
  const List<double> colorStops = <double>[0.0, 0.05, 0.4, 0.6, 0.9, 1.0];

  test('Should draw with radial gradient with TileMode.repeated.', () async {
    Rect shaderRect = const Rect.fromLTRB(50, 50, 100, 100);
    await _testGradient(
        'radial_gradient_tilemode_repeated',
        Gradient.radial(
            Offset((shaderRect.left + shaderRect.right) / 2,
                (shaderRect.top + shaderRect.bottom) / 2),
            shaderRect.width / 2,
            colors,
            colorStops,
            TileMode.repeated),
        shaderRect: shaderRect);
  });

  test('Should draw with radial gradient with TileMode.mirrored.', () async {
    Rect shaderRect = const Rect.fromLTRB(50, 50, 100, 100);
    await _testGradient(
        'radial_gradient_tilemode_mirror',
        Gradient.radial(
            Offset((shaderRect.left + shaderRect.right) / 2,
                (shaderRect.top + shaderRect.bottom) / 2),
            shaderRect.width / 2,
            colors,
            colorStops,
            TileMode.mirror),
        shaderRect: shaderRect);
  });
}
