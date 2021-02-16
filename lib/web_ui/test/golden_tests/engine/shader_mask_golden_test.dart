// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:html' as html;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/ui.dart';
import 'package:ui/src/engine.dart' hide ClipRectEngineLayer, BackdropFilterEngineLayer;

import 'package:web_engine_tester/golden_tester.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() async {
  setUp(() async {
    debugShowClipLayers = true;
    SurfaceSceneBuilder.debugForgetFrameScene();
    for (html.Node scene in html.document.querySelectorAll('flt-scene')) {
      scene.remove();
    }
    initWebGl();
    await webOnlyInitializePlatform();
    webOnlyFontCollection.debugRegisterTestFonts();
    await webOnlyFontCollection.ensureFontsLoaded();
  });

  void _renderScene(BlendMode blendMode) {
    final Rect region = Rect.fromLTWH(0, 0, 400, 400);

    final SurfaceSceneBuilder builder = SurfaceSceneBuilder();
    final Picture circles1 = _drawTestPictureWithCircles(region, 30, 30);
    builder.addPicture(Offset.zero, circles1);

    List<Color> colors = <Color>[
      Color(0xFF000000),
      Color(0xFFFF3C38),
      Color(0xFFFF8C42),
      Color(0xFFFFF275),
      Color(0xFF6699CC),
      Color(0xFF656D78),];
    List<double> stops = <double>[0.0, 0.05, 0.4, 0.6, 0.9, 1.0];

    EngineGradient shader = GradientLinear(Offset(0, 0), Offset(120, 120),
        colors, stops, TileMode.clamp,
        Matrix4.identity().storage);

    builder.pushShaderMask(shader, Rect.fromLTWH(200, 30, 120, 120),
        blendMode, oldLayer: null);
    final Picture circles2 = _drawTestPictureWithCircles(region, 200, 30);
    builder.addPicture(Offset.zero, circles2);
    builder.pop();

    html.document.body.append(builder
        .build()
        .webOnlyRootElement);
  }

  test('Renders shader mask with linear gradient BlendMode color', () async {
    _renderScene(BlendMode.color);
    await matchGoldenFile('shadermask_linear_color.png',
        region: Rect.fromLTWH(0, 0, 360, 200));
    // TODO: Implement workaround for webkit bug not applying svg color filter.
  }, skip: browserEngine == BrowserEngine.webkit);

  test('Renders shader mask with linear gradient BlendMode xor', () async {
    _renderScene(BlendMode.xor);
    await matchGoldenFile('shadermask_linear_xor.png',
        region: Rect.fromLTWH(0, 0, 360, 200));
    // TODO: Implement workaround for webkit bug not applying svg xor filter.
  }, skip: browserEngine == BrowserEngine.webkit);

  test('Renders shader mask with linear gradient BlendMode plus', () async {
    _renderScene(BlendMode.plus);
    await matchGoldenFile('shadermask_linear_plus.png',
        region: Rect.fromLTWH(0, 0, 360, 200));
    // TODO: Implement workaround for webkit bug not applying svg plus filter.
  }, skip: browserEngine == BrowserEngine.webkit);

  test('Renders shader mask with linear gradient BlendMode modulate', () async {
    _renderScene(BlendMode.modulate);
    await matchGoldenFile('shadermask_linear_modulate.png',
        region: Rect.fromLTWH(0, 0, 360, 200));
  });

  test('Renders shader mask with linear gradient BlendMode overlay', () async {
    _renderScene(BlendMode.overlay);
    await matchGoldenFile('shadermask_linear_overlay.png',
        region: Rect.fromLTWH(0, 0, 360, 200));
  });

  test('Renders shader mask with linear gradient BlendMode src', () async {
    _renderScene(BlendMode.src);
    await matchGoldenFile('shadermask_linear.png',
        region: Rect.fromLTWH(0, 0, 360, 200));
  });
}

Picture _drawTestPictureWithCircles(Rect region, double offsetX, double offsetY) {
  final EnginePictureRecorder recorder = PictureRecorder();
  final RecordingCanvas canvas =
  recorder.beginRecording(region);
  canvas.drawCircle(
      Offset(offsetX + 10, offsetY + 10), 30,
      Paint()..style = PaintingStyle.fill);
  canvas.drawCircle(
      Offset(offsetX + 90, offsetY + 10), 30,
      Paint()
        ..style = PaintingStyle.fill
        ..color = const Color(0xFFFF0000));
  canvas.drawCircle(
      Offset(offsetX + 10, offsetY + 90), 30,
      Paint()
        ..style = PaintingStyle.fill
        ..color = const Color(0xFF00FF00));
  canvas.drawCircle(
      Offset(offsetX + 90, offsetY + 90), 30,
      Paint()
        ..style = PaintingStyle.fill
        ..color = const Color(0xFF0000FF));
  return recorder.endRecording();
}
