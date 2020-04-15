// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:html' as html;
import 'dart:math' as math;

import 'package:ui/ui.dart';
import 'package:ui/src/engine.dart';
import 'package:test/test.dart';

import '../../matchers.dart';
import 'package:web_engine_tester/golden_tester.dart';

final Rect region = Rect.fromLTWH(0, 0, 500, 100);

void main() async {
  setUp(() async {
    debugShowClipLayers = true;
    SurfaceSceneBuilder.debugForgetFrameScene();
    for (html.Node scene in html.document.querySelectorAll('flt-scene')) {
      scene.remove();
    }

    await webOnlyInitializePlatform();
    webOnlyFontCollection.debugRegisterTestFonts();
    await webOnlyFontCollection.ensureFontsLoaded();
  });

  // The black circle on the left should not be blurred since it is outside
  // the clip boundary around backdrop filter. However there should be only
  // one red dot since the other one should be blurred by filter.
  test('Background should only blur at ancestor clip boundary', () async {
    final SurfaceSceneBuilder builder = SurfaceSceneBuilder();
    _drawBackground(builder);
    builder.pushClipRect(
      const Rect.fromLTRB(10, 10, 300, 300),
    );
    _drawTestPictureWithCircles(builder, 30, 30);
    builder.pushClipRect(
      const Rect.fromLTRB(60, 10, 300, 300),
    );
    builder.pushBackdropFilter(ImageFilter.blur(sigmaX: 10.0, sigmaY: 10.0),
      oldLayer: null);
    _drawTestPictureWithCircles(builder, 90, 30);
    builder.pop();
    builder.pop();
    builder.pop();

    html.document.body.append(builder
        .build()
        .webOnlyRootElement);

    await matchGoldenFile('backdrop_filter_clip.png', region: region);
  });
}

void _drawTestPictureWithCircles(SceneBuilder builder, double offsetX,
    double offsetY) {
  final EnginePictureRecorder recorder = PictureRecorder();
  final RecordingCanvas canvas =
  recorder.beginRecording(const Rect.fromLTRB(0, 0, 400, 400));
  canvas.drawCircle(
      Offset(offsetX + 10, offsetY + 10), 10, Paint()..style = PaintingStyle.fill);
  canvas.drawCircle(
      Offset(offsetX + 60, offsetY + 10),
      10,
      Paint()
        ..style = PaintingStyle.fill
        ..color = const Color.fromRGBO(255, 0, 0, 1));
  canvas.drawCircle(
      Offset(offsetX + 10, offsetY + 60),
      10,
      Paint()
        ..style = PaintingStyle.fill
        ..color = const Color.fromRGBO(0, 255, 0, 1));
  canvas.drawCircle(
      Offset(offsetX + 60, offsetY + 60),
      10,
      Paint()
        ..style = PaintingStyle.fill
        ..color = const Color.fromRGBO(0, 0, 255, 1));
  final Picture picture = recorder.endRecording();
  builder.addPicture(
    Offset.zero,
    picture,
  );
}

void _drawBackground(SceneBuilder builder) {
  final EnginePictureRecorder recorder = PictureRecorder();
  final RecordingCanvas canvas =
  recorder.beginRecording(const Rect.fromLTRB(0, 0, 400, 400));
  canvas.drawRect(
      Rect.fromLTWH(8, 8, 400.0 - 16, 400.0 - 16),
      Paint()
        ..style = PaintingStyle.fill
        ..color = Color(0xFFE0FFE0)
      );
  final Picture picture = recorder.endRecording();
  builder.addPicture(
    Offset.zero,
    picture,
  );
}
