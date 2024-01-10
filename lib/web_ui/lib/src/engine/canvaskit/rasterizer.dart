// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

abstract class Rasterizer {
  /// Creates a [ViewRasterizer] for a given [view].
  ViewRasterizer createViewRasterizer(EngineFlutterView view);

  /// Sets the maximum size of the resource cache to [bytes].
  void setResourceCacheMaxBytes(int bytes);

  /// Disposes this rasterizer and all [ViewRasterizer]s that it created.
  void dispose();
}

abstract class ViewRasterizer {
  ViewRasterizer(this.view);

  /// The view this rasterizer renders into.
  final EngineFlutterView view;

  /// The size of the current frame being rasterized.
  ui.Size currentFrameSize = ui.Size.zero;

  /// The context which is persisted between frames.
  final CompositorContext context = CompositorContext();

  /// The platform view embedder.
  late final HtmlViewEmbedder viewEmbedder = HtmlViewEmbedder(sceneHost, this);

  /// A factory for creating overlays.
  OverlayCanvasFactory<OverlayCanvas> get overlayFactory;

  /// The scene host which this rasterizer should raster into.
  DomElement get sceneHost => view.dom.sceneHost;

  /// Draws the [layerTree] to the screen for the view associated with this
  /// rasterizer.
  Future<void> draw(LayerTree layerTree) async {
    final ui.Size frameSize = view.physicalSize;
    if (frameSize.isEmpty) {
      // Available drawing area is empty. Skip drawing.
      return;
    }

    currentFrameSize = frameSize;
    prepareToDraw();
    viewEmbedder.frameSize = currentFrameSize;
    final CkPictureRecorder pictureRecorder = CkPictureRecorder();
    pictureRecorder.beginRecording(ui.Offset.zero & currentFrameSize);
    pictureRecorder.recordingCanvas!.clear(const ui.Color(0x00000000));
    final Frame compositorFrame =
        context.acquireFrame(pictureRecorder.recordingCanvas!, viewEmbedder);

    compositorFrame.raster(layerTree, ignoreRasterCache: true);

    sceneHost.prepend(overlayFactory.baseCanvas.htmlElement);
    await rasterizeToCanvas(
        overlayFactory.baseCanvas, <CkPicture>[pictureRecorder.endRecording()]);

    await viewEmbedder.submitFrame();
  }

  /// Do some initialization to prepare to draw a frame.
  ///
  /// For example, in the [OffscreenCanvasRasterizer], this ensures the backing
  /// [OffscreenCanvas] is the correct size to draw the frame.
  void prepareToDraw();

  /// Rasterize the [pictures] to the given [canvas].
  Future<void> rasterizeToCanvas(
      OverlayCanvas canvas, List<CkPicture> pictures);

  /// Get a [OverlayCanvas] to use as an overlay.
  OverlayCanvas getOverlay() {
    return overlayFactory.getCanvas();
  }

  /// Release the given [overlay] so it may be reused.
  void releaseOverlay(OverlayCanvas overlay) {
    overlayFactory.releaseCanvas(overlay);
  }

  /// Release all overlays.
  void releaseOverlays() {
    overlayFactory.releaseCanvases();
  }

  /// Remove all overlays that have been created from the DOM.
  void removeOverlaysFromDom() {
    overlayFactory.removeSurfacesFromDom();
  }

  /// Disposes this rasterizer.
  void dispose();
}

abstract class OverlayCanvas {
  DomElement get htmlElement;

  /// Whether or not this overlay canvas is attached to the DOM.
  bool get isConnected;

  /// Initialize the overlay.
  void initialize();

  /// Disposes this overlay.
  void dispose();
}
