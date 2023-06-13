// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// A class that can rasterize [LayerTree]s into a given [Surface].
class Rasterizer {
  final CompositorContext context = CompositorContext();
  final List<ui.VoidCallback> _postFrameCallbacks = <ui.VoidCallback>[];

  /// This is an SkSurface backed by an OffScreenCanvas. This single Surface is
  /// used to render to many RenderCanvases to produce the rendered scene.
  final Surface _offscreenSurface = Surface();
  ui.Size _currentFrameSize = ui.Size.zero;
  late SurfaceFrame _currentFrame;

  /// Render the given [picture] so it is displayed by the given [canvas].
  void rasterizeToCanvas(RenderCanvas canvas, CkPicture picture) {
    // Ensure the [canvas] is the correct size.
    canvas.ensureSize(_currentFrameSize);

    final CkCanvas skCanvas = _currentFrame.skiaCanvas;
    skCanvas.clear(const ui.Color(0x00000000));
    skCanvas.drawPicture(picture);
    _currentFrame.submit();

    final DomImageBitmap bitmap =
        _offscreenSurface.offscreenCanvas!.transferToImageBitmap();
    canvas.renderContext!.transferFromImageBitmap(bitmap);
  }

  /// Sets the maximum size of the Skia resource cache, in bytes.
  void setSkiaResourceCacheMaxBytes(int bytes) =>
      _offscreenSurface.setSkiaResourceCacheMaxBytes(bytes);

  /// Creates a new frame from this rasterizer's surface, draws the given
  /// [LayerTree] into it, and then submits the frame.
  void draw(LayerTree layerTree) {
    try {
      if (layerTree.frameSize.isEmpty) {
        // Available drawing area is empty. Skip drawing.
        return;
      }

      _currentFrameSize = layerTree.frameSize;
      _currentFrame = _offscreenSurface.acquireFrame(_currentFrameSize);
      RenderCanvasFactory.instance.baseCanvas.ensureSize(_currentFrameSize);
      HtmlViewEmbedder.instance.frameSize = _currentFrameSize;
      final CkPictureRecorder pictureRecorder = CkPictureRecorder();
      pictureRecorder.beginRecording(ui.Offset.zero & _currentFrameSize);
      pictureRecorder.recordingCanvas!.clear(const ui.Color(0x00000000));
      final Frame compositorFrame = context.acquireFrame(
          pictureRecorder.recordingCanvas!, HtmlViewEmbedder.instance);

      compositorFrame.raster(layerTree, ignoreRasterCache: true);

      RenderCanvasFactory.instance.baseCanvas.addToScene();
      rasterizeToCanvas(RenderCanvasFactory.instance.baseCanvas,
          pictureRecorder.endRecording());

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
