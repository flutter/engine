// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../window.dart';
import 'renderer.dart';
import 'util.dart';

/// A visible (on-screen) canvas that can display bitmaps produced by CanvasKit
/// in the (off-screen) SkSurface which is backed by an OffscreenCanvas.
///
/// In a typical frame, the content will be rendered via CanvasKit in an
/// OffscreenCanvas, and then the contents will be transferred to the
/// RenderCanvas via `transferFromImageBitmap()`.
///
/// If we need more RenderCanvases, for example in the case where there are
/// platform views and we need overlays to render the frame correctly, then
/// we will create multiple RenderCanvases, but crucially still only have
/// one OffscreenCanvas which transfers bitmaps to all of the RenderCanvases.
///
/// To render into the OffscreenCanvas with CanvasKit we need to create a
/// WebGL context, which is not only expensive, but the browser has a limit
/// on the maximum amount of WebGL contexts which can be live at once. Using
/// a single OffscreenCanvas and multiple RenderCanvases allows us to only
/// create a single WebGL context.
class RenderCanvas {
  RenderCanvas();

  /// The root HTML element for this canvas.
  ///
  /// This element contains the canvas used to draw the UI. Unlike the canvas,
  /// this element is permanent. It is never replaced or deleted, until this
  /// canvas is disposed of via [dispose].
  ///
  /// Conversely, the canvas that lives inside this element can be swapped, for
  /// example, when the screen size changes, or when the WebGL context is lost
  /// due to the browser tab becoming dormant.
  final DomElement htmlElement = createDomElement('flt-canvas-container');

  /// The underlying `<canvas>` element used to display the pixels.
  DomCanvasElement? canvasElement;
  int _pixelWidth = -1;
  int _pixelHeight = -1;

  DomCanvasBitmapRendererContext? renderContext;

  ui.Size? _currentCanvasPhysicalSize;
  ui.Size? _currentRenderSize;
  double _currentDevicePixelRatio = -1;

  bool _addedToScene = false;
  void addToScene() {
    if (!_addedToScene) {
      CanvasKitRenderer.instance.sceneHost!.prepend(htmlElement);
    }
    _addedToScene = true;
  }

  /// Sets the CSS size of the canvas so that canvas pixels are 1:1 with device
  /// pixels.
  ///
  /// The logical size of the canvas is not based on the size of the window
  /// but on the size of the canvas, which, due to `ceil()` above, may not be
  /// the same as the window. We do not round/floor/ceil the logical size as
  /// CSS pixels can contain more than one physical pixel and therefore to
  /// match the size of the window precisely we use the most precise floating
  /// point value we can get.
  void _updateLogicalHtmlCanvasSize() {
    final double logicalWidth = _pixelWidth / window.devicePixelRatio;
    final double logicalHeight = _pixelHeight / window.devicePixelRatio;
    final DomCSSStyleDeclaration style = canvasElement!.style;
    style.width = '${logicalWidth}px';
    style.height = '${logicalHeight}px';
  }

  /// This function is expensive.
  ///
  /// It's better to reuse canvas if possible.
  void _createNewCanvas(ui.Size physicalSize) {
    // Clear the container, if it's not empty. We're going to create a new <canvas>.
    if (canvasElement != null) {
      canvasElement!.remove();
    }

    // If `physicalSize` is not precise, use a slightly bigger canvas. This way
    // we ensure that the rendred picture covers the entire browser window.
    _pixelWidth = physicalSize.width.ceil();
    _pixelHeight = physicalSize.height.ceil();
    final DomCanvasElement htmlCanvas = createDomCanvasElement(
      width: _pixelWidth,
      height: _pixelHeight,
    );
    canvasElement = htmlCanvas;
    renderContext = htmlCanvas.bitmapRendererContext;

    // The DOM elements used to render pictures are used purely to put pixels on
    // the screen. They have no semantic information. If an assistive technology
    // attempts to scan picture content it will look like garbage and confuse
    // users. UI semantics are exported as a separate DOM tree rendered parallel
    // to pictures.
    //
    // Why are layer and scene elements not hidden from ARIA? Because those
    // elements may contain platform views, and platform views must be
    // accessible.
    htmlCanvas.setAttribute('aria-hidden', 'true');

    htmlCanvas.style.position = 'absolute';
    _updateLogicalHtmlCanvasSize();
    htmlElement.append(htmlCanvas);
  }

  /// Ensures that this canvas can draw a frame of the given [size].
  void ensureSize(ui.Size size) {
    if (size.isEmpty) {
      throw CanvasKitError('Cannot create canvases of empty size.');
    }

    // Check if the frame is the same size as before, and if so, don't allocate
    // a new canvas as the previous canvas is big enough to fit everything.
    final ui.Size? previousRenderSize = _currentRenderSize;
    if (previousRenderSize != null &&
        size.width == previousRenderSize.width &&
        size.height == previousRenderSize.height) {
      // The existing canvas doesn't need to be resized.
      if (window.devicePixelRatio != _currentDevicePixelRatio) {
        _updateLogicalHtmlCanvasSize();
      }
      return;
    }

    final ui.Size? previousCanvasSize = _currentCanvasPhysicalSize;
    // If the canvas is too large or too small, resize it to the exact size of
    // the frame. We cannot allow the canvas to be larger than the screen
    // because then when we call `transferFromImageBitmap()` the bitmap will
    // be scaled to cover the entire canvas.
    if (previousCanvasSize != null) {
      canvasElement!.width = size.width;
      canvasElement!.height = size.height;
      _currentCanvasPhysicalSize = size;
      _pixelWidth = size.width.ceil();
      _pixelHeight = size.height.ceil();
      _updateLogicalHtmlCanvasSize();
    }

    // This is the first frame we have rendered with this canvas.
    if (_currentCanvasPhysicalSize == null) {
      _addedToScene = false;

      _createNewCanvas(size);
      _currentCanvasPhysicalSize = size;
    } else if (window.devicePixelRatio != _currentDevicePixelRatio) {
      _updateLogicalHtmlCanvasSize();
    }

    _currentDevicePixelRatio = window.devicePixelRatio;
    _currentRenderSize = size;
  }

  void dispose() {
    htmlElement.remove();
  }
}
