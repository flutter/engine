// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// A Rasterizer which uses one or many on-screen WebGL contexts to display the
/// scene. This way of rendering is prone to bugs because there is a limit to
/// how many WebGL contexts can be live at one time as well as bugs in sharing
/// GL resources between the contexts. However, using [createImageBitmap] is
/// currently very slow on Firefox and Safari browsers, so directly rendering
/// to several
class MultiSurfaceRasterizer extends Rasterizer {
  @override
  MultiSurfaceViewRasterizer createViewRasterizer(EngineFlutterView view) {
    return MultiSurfaceViewRasterizer(view, this);
  }

  @override
  void dispose() {
    // TODO(harryterkelsen): implement dispose
  }

  @override
  void setResourceCacheMaxBytes(int bytes) {
    // TODO(harryterkelsen): implement setResourceCacheMaxBytes
  }
}

class MultiSurfaceViewRasterizer extends ViewRasterizer {
  MultiSurfaceViewRasterizer(super.view, this.rasterizer);

  final MultiSurfaceRasterizer rasterizer;

  @override
  final OverlayCanvasFactory<Surface> overlayFactory =
      OverlayCanvasFactory<Surface>(
          createCanvas: () => Surface(useOffscreenCanvas: false));

  @override
  void dispose() {
    // TODO: implement dispose
  }

  @override
  void prepareToDraw() {
    overlayFactory.baseCanvas.ensureSurface(currentFrameSize);
  }

  @override
  Future<void> rasterizeToCanvas(
      OverlayCanvas canvas, List<CkPicture> pictures) {
    final Surface surface = canvas as Surface;
    surface.ensureSurface(currentFrameSize);
    final CkCanvas skCanvas = surface.getCanvas();
    skCanvas.clear(const ui.Color(0x00000000));
    pictures.forEach(skCanvas.drawPicture);
    surface.flush();
    return Future<void>.value();
  }
}
