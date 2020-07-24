// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:html' as html;

import 'package:ui/ui.dart';
import 'package:ui/src/engine.dart';
import 'package:test/test.dart';

import 'package:web_engine_tester/golden_tester.dart';

void main() async {
  final Rect region = Rect.fromLTWH(0, 0, 500, 500);

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

  test('Convert Picture to Image', () async {
    final SurfaceSceneBuilder builder = SurfaceSceneBuilder();
    final Image testImage = await _drawTestPictureWithCircle(region);
    builder.addImage(Offset.zero, testImage);

    html.document.body.append(builder
        .build()
        .webOnlyRootElement);

    await matchGoldenFile('picture_to_image.png', region: region, write: true);
  });
}

Future<Image> _drawTestPictureWithCircle(Rect region) async {
  final EnginePictureRecorder recorder = PictureRecorder();
  final RecordingCanvas canvas = recorder.beginRecording(region);
  canvas.drawOval(
      region,
      Paint()
        ..style = PaintingStyle.fill
        ..color = Color(0xFF00FF00);
  final Picture picture = pictureRecorder.endRecording();
  return picture.toImage(region.width, region.height);
}
