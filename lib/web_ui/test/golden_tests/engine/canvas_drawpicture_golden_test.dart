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

  test('draw picture', () async {
    final SurfaceSceneBuilder builder = SurfaceSceneBuilder();
    builder.pushClipRect(
      const Rect.fromLTRB(0, 0, 100, 100),
    );
    _drawTestPicture(builder);
    builder.pop();

    html.document.body.append(builder
        .build()
        .webOnlyRootElement);

    await matchGoldenFile('canvas_drawpicture.png', region: region, write: true);
  });

//  test('draw growing picture across frames', () async {
//    final SurfaceSceneBuilder builder = SurfaceSceneBuilder();
//
//    builder.pushOffset(0, 60);
//    builder.pushTransform(
//      Matrix4
//          .diagonal3Values(1, -1, 1)
//          .storage,
//    );
//    builder.pushClipRect(
//      const Rect.fromLTRB(10, 10, 60, 60),
//    );
//    _drawTestPicture(builder);
//    builder.pop();
//    builder.pop();
//    builder.pop();
//
//    html.document.body.append(builder
//        .build()
//        .webOnlyRootElement);
//
//    // Now draw picture again but at larger size.
//
//    await matchGoldenFile('canvas_drawpicture_acrossframes.png',
//        region: region);
//  });
}

void _drawTestPicture(SceneBuilder builder) {
  final HtmlImage image = _createRealTestImage();
  final EnginePictureRecorder recorder = PictureRecorder();
  final RecordingCanvas canvas =
  recorder.beginRecording(const Rect.fromLTRB(0, 0, 100, 100));
  canvas.debugEnforceArbitraryPaint();
  canvas.clipRRect(RRect.fromLTRBR(0, 0, 10, 10, Radius.circular(4)));
  canvas.drawImageRect(image, Rect.fromLTWH(0, 0, 20, 20), Rect.fromLTWH(0, 0, 10, 10), Paint());
  final Picture picture = recorder.endRecording();
  builder.addPicture(
    Offset.zero,
    picture,
  );
}

typedef PaintCallback = void Function(RecordingCanvas canvas);

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
