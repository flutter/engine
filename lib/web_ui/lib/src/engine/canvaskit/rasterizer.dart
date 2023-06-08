// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import '../frame_reference.dart';
import 'canvas.dart';
import 'embedded_views.dart';
import 'layer_tree.dart';
import 'render_canvas_factory.dart';
import 'surface.dart';

/// A class that can rasterize [LayerTree]s into a given [Surface].
class Rasterizer {
  final CompositorContext context = CompositorContext();
  final List<ui.VoidCallback> _postFrameCallbacks = <ui.VoidCallback>[];

  /// This is an SkSurface backed by an OffScreenCanvas. This single Surface is
  /// used to render to many RenderCanvases to produce the rendered scene.
  final Surface offscreenSurface = Surface();

  void setSkiaResourceCacheMaxBytes(int bytes) =>
      offscreenSurface.setSkiaResourceCacheMaxBytes(bytes);

  /// Creates a new frame from this rasterizer's surface, draws the given
  /// [LayerTree] into it, and then submits the frame.
  void draw(LayerTree layerTree) {
    try {
      if (layerTree.frameSize.isEmpty) {
        // Available drawing area is empty. Skip drawing.
        return;
      }

      SurfaceFrame frame = offscreenSurface.acquireFrame(layerTree.frameSize);
      RenderCanvasFactory.instance.baseCanvas.ensureSize(layerTree.frameSize);
      HtmlViewEmbedder.instance.frameSize = layerTree.frameSize;
      final CkPictureRecorder pictureRecorder = CkPictureRecorder();
      pictureRecorder.beginRecording(ui.Offset.zero & layerTree.frameSize);
      pictureRecorder.recordingCanvas!.clear(const ui.Color(0x00000000));
      final Frame compositorFrame =
          context.acquireFrame(pictureRecorder.recordingCanvas!, HtmlViewEmbedder.instance);

      compositorFrame.raster(layerTree, ignoreRasterCache: true);
      RenderCanvasFactory.instance.baseCanvas.addToScene();

      final CkCanvas canvas = frame.skiaCanvas;
      canvas.clear(const ui.Color(0x00000000));
      canvas.drawPicture(pictureRecorder.endRecording());
      frame.submit();
      final DomImageBitmap bitmap = CanvasKitRenderer.instance.rasterizer.offscreenSurface.offscreenCanvas!.transferToImageBitmap();
      RenderCanvasFactory.instance.baseCanvas.renderContext!.transferFromImageBitmap(bitmap);
      HtmlViewEmbedder.instance.submitFrame();
    } finally {
      _runPostFrameCallbacks();
    }
  }

  void addPostFrameCallback(ui.VoidCallback callback) {
    _postFrameCallbacks.add(callback);
  }

  void _runPostFrameCallbacks() {
    for (int i = 0; i < _postFrameCallbacks.length; i++) {
      final ui.VoidCallback callback = _postFrameCallbacks[i];
      callback();
    }
    for (int i = 0; i < frameReferences.length; i++) {
      frameReferences[i].value = null;
    }
    frameReferences.clear();
  }

  /// Forces the post-frame callbacks to run. Useful in tests.
  @visibleForTesting
  void debugRunPostFrameCallbacks() {
    _runPostFrameCallbacks();
  }
}
