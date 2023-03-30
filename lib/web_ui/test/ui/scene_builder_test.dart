// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:web_engine_tester/golden_tester.dart';

import 'utils.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUiTest();

  group('${ui.SceneBuilder}', () {
    const ui.Rect region = ui.Rect.fromLTWH(0, 0, 300, 300);
    test('Test offset layer', () async {
      final ui.SceneBuilder sceneBuilder = ui.SceneBuilder();
      sceneBuilder.pushOffset(150, 150);
      sceneBuilder.addPicture(ui.Offset.zero, drawPicture((ui.Canvas canvas) {
        canvas.drawCircle(
          ui.Offset.zero,
          50,
          ui.Paint()..color = const ui.Color(0xFF4CAF50) // Colors.green
        );
      }));
      await renderer.renderScene(sceneBuilder.build());

      await matchGoldenFile('scene_builder_centered_circle.png', region: region);
    });
  });
}

ui.Picture drawPicture(void Function(ui.Canvas) drawCommands) {
  final ui.PictureRecorder recorder = ui.PictureRecorder();
  final ui.Canvas canvas = ui.Canvas(recorder);
  drawCommands(canvas);
  return recorder.endRecording();
}
