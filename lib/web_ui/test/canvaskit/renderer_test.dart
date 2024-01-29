// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

class TestRasterizer extends Rasterizer {
  TestViewRasterizer? viewRasterizer;

  @override
  TestViewRasterizer createViewRasterizer(EngineFlutterView view) {
    return viewRasterizer ??= TestViewRasterizer(view);
  }

  @override
  void dispose() {
    // Do nothing
  }

  @override
  void setResourceCacheMaxBytes(int bytes) {
    // Do nothing
  }

  List<LayerTree> get treesRendered => viewRasterizer!.treesRendered;
}

class TestViewRasterizer extends ViewRasterizer {
  TestViewRasterizer(super.view);

  List<LayerTree> treesRendered = <LayerTree>[];

  @override
  DisplayCanvasFactory<DisplayCanvas> get displayFactory =>
      throw UnimplementedError();

  @override
  void prepareToDraw() {
    // Do nothing
  }

  @override
  Future<void> draw(LayerTree tree) async {
    treesRendered.add(tree);
    return Future<void>.value();
  }

  @override
  Future<void> rasterizeToCanvas(
      DisplayCanvas canvas, List<CkPicture> pictures) {
    // No-op
    return Future<void>.value();
  }
}

void testMain() {
  group('Renderer', () {
    setUpCanvasKitTest();

    test('always renders most recent picture and skips intermediate pictures',
        () async {
      final TestRasterizer testRasterizer = TestRasterizer();
      CanvasKitRenderer.instance.debugOverrideRasterizer(testRasterizer);

      // Create another view to render into to force the renderer to make
      // a [ViewRasterizer] for it.
      final EngineFlutterView testView = EngineFlutterView(
          EnginePlatformDispatcher.instance, createDomElement('test-view'));
      EnginePlatformDispatcher.instance.viewManager.registerView(testView);

      final List<LayerTree> treesToRender = <LayerTree>[];
      final List<Future<void>> renderFutures = <Future<void>>[];
      for (int i = 1; i < 20; i++) {
        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder);
        canvas.drawRect(const ui.Rect.fromLTWH(0, 0, 50, 50),
            ui.Paint()..color = const ui.Color(0xff00ff00));
        final ui.Picture picture = recorder.endRecording();
        final ui.SceneBuilder builder = ui.SceneBuilder();
        builder.addPicture(ui.Offset.zero, picture);
        final ui.Scene scene = builder.build();
        treesToRender.add((scene as LayerScene).layerTree);
        renderFutures
            .add(CanvasKitRenderer.instance.renderScene(scene, testView));
      }
      await Future.wait(renderFutures);

      // Should just render the first and last pictures and skip the one inbetween.
      expect(testRasterizer.treesRendered.length, 2);
      expect(testRasterizer.treesRendered.first, treesToRender.first);
      expect(testRasterizer.treesRendered.last, treesToRender.last);
    });
  });
}
