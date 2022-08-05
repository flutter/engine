// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:web_engine_tester/golden_tester.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('CanvasKit', () {
    setUpCanvasKitTest();

    // Regression test for https://github.com/flutter/flutter/issues/63715
    test('TransformLayer prerolls correctly', () async {
      final CkPicture picture =
          paintPicture(const ui.Rect.fromLTRB(0, 0, 60, 60), (CkCanvas canvas) {
        canvas.drawRect(const ui.Rect.fromLTRB(0, 0, 60, 60),
            CkPaint()..style = ui.PaintingStyle.fill);
      });

      final LayerSceneBuilder sb = LayerSceneBuilder();
      sb.pushClipRect(const ui.Rect.fromLTRB(15, 15, 30, 30));

      // Intentionally use a perspective transform, which triggered the
      // https://github.com/flutter/flutter/issues/63715 bug.
      sb.pushTransform(
          Float64List.fromList(Matrix4.identity().storage
            ..[15] = 2,
      ));

      sb.addPicture(ui.Offset.zero, picture);
      final LayerTree layerTree = sb.build().layerTree;
      CanvasKitRenderer.instance.rasterizer.draw(layerTree);
      final ClipRectEngineLayer clipRect = layerTree.rootLayer.debugLayers.single as ClipRectEngineLayer;
      expect(clipRect.paintBounds, const ui.Rect.fromLTRB(15, 15, 30, 30));

      final TransformEngineLayer transform = clipRect.debugLayers.single as TransformEngineLayer;
      expect(transform.paintBounds, const ui.Rect.fromLTRB(0, 0, 30, 30));
    });

    test('can push a leaf layer without a container layer', () async {
      final CkPictureRecorder recorder = CkPictureRecorder();
      recorder.beginRecording(ui.Rect.zero);
      LayerSceneBuilder().addPicture(ui.Offset.zero, recorder.endRecording());
    });

    test('different layer transform kinds render correctly', () async {
      const ui.Rect region = ui.Rect.fromLTRB(0, 0, 400, 100);
      final CkPictureRecorder recorder = CkPictureRecorder();
      final CkCanvas canvas = recorder.beginRecording(region);

      for (int row = 0; row < 6; row++) {
        for (int column = 0; column < 3; column++) {
          const double sideLength = 18;
          final ui.Rect rect = ui.Rect.fromLTWH(
            row.isEven
                ? (column * 2) * sideLength
                : (column * 2 + 1) * sideLength,
            row * sideLength,
            sideLength,
            sideLength,
          );
          canvas.drawRect(rect, CkPaint()..color = const ui.Color(0xffff0000));
        }
      }
      final CkPicture checkerboard = recorder.endRecording();

      final LayerSceneBuilder builder = LayerSceneBuilder();
      builder.pushOffset(50, 50);

      // Paint with zero offset
      builder.pushOffset(0, 0);
      builder.addPicture(ui.Offset.zero, checkerboard);
      builder.pop();

      // Paint with non-zero offset
      builder.pushOffset(150, 0);
      builder.addPicture(ui.Offset.zero, checkerboard);
      builder.pop();

      // Paint with identity transform
      builder.pushOffset(300, 0);
      builder.pushTransform(Matrix4.identity().debugToFloat64List());
      builder.addPicture(ui.Offset.zero, checkerboard);
      builder.pop();
      builder.pop();

      // Paint with 2D translation transform
      builder.pushOffset(450, 0);
      builder.pushTransform(Matrix4.translationValues(1, 20, 0).debugToFloat64List());
      builder.addPicture(ui.Offset.zero, checkerboard);
      builder.pop();
      builder.pop();

      // Paint with general transform
      builder.pushOffset(600, 0);
      builder.pushTransform(Matrix4.rotationZ(0.2).debugToFloat64List());
      builder.addPicture(ui.Offset.zero, checkerboard);
      builder.pop();
      builder.pop();

      builder.pop();
      EnginePlatformDispatcher.instance.rasterizer!.draw(builder.build().layerTree);
      await matchGoldenFile('canvaskit_transform_layers.png', region: region);
    });
  });
}
