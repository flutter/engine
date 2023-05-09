// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

import 'package:ui/ui.dart' as ui;
import 'package:web_engine_tester/golden_tester.dart';

import '../common/test_initialization.dart';
import 'utils.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUnitTests(
    setUpTestViewDimensions: false,
  );

  const ui.Rect drawRegion = ui.Rect.fromLTWH(0, 0, 300, 300);
  const ui.Rect imageRegion = ui.Rect.fromLTWH(0, 0, 150, 150);

  // Emits a set of rendering tests for an image
  // `imageGenerator` should produce an image that is 150x150 pixels.
  void emitImageTests(String name, Future<ui.Image> Function() imageGenerator) {
    group(name, () {
      test('drawImage', () async {
        final ui.Image image = await imageGenerator();
        expect(image.width, 150);
        expect(image.height, 150);

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawImage(image, const ui.Offset(100, 100), ui.Paint());

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_canvas_drawImage.png', region: drawRegion);
      });

      test('drawImageRect', () async {
        final ui.Image image = await imageGenerator();
        expect(image.width, 150);
        expect(image.height, 150);

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawImageRect(
          image,
          const ui.Rect.fromLTRB(50, 50, 100, 100),
          const ui.Rect.fromLTRB(100, 100, 150, 150),
          ui.Paint()
        );

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_canvas_drawImageRect.png', region: drawRegion);
      });

      test('drawImageNine', () async {
        final ui.Image image = await imageGenerator();
        expect(image.width, 150);
        expect(image.height, 150);

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawImageNine(
          image,
          const ui.Rect.fromLTRB(50, 50, 100, 100),
          drawRegion,
          ui.Paint()
        );

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_canvas_drawImageNine.png', region: drawRegion);
      });
    });
  }

  emitImageTests('picture_toImage', () {
    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Canvas canvas = ui.Canvas(recorder, imageRegion);
    for (int y = 0; y < 15; y++) {
      for (int x = 0; x < 15; x++) {
        final ui.Offset center = ui.Offset(x * 10 + 5, y * 10 + 5);
        final ui.Color color = ui.Color.fromRGBO(
          (center.dx * 256 / 150).round(),
          (center.dy * 256 / 150).round(), 0, 1);
        canvas.drawCircle(center, 5, ui.Paint()..color = color);
      }
    }
    return recorder.endRecording().toImage(150, 150);
  });
}
