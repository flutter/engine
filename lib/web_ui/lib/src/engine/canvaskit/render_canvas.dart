// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:js_interop';

import 'package:ui/ui.dart' as ui;

import '../browser_detection.dart';
import '../configuration.dart';
import '../dom.dart';
import '../platform_dispatcher.dart';
import '../util.dart';
import '../window.dart';
import 'canvas.dart';
import 'canvaskit_api.dart';
import 'renderer.dart';
import 'surface_factory.dart';
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

  ui.Size? _currentCanvasPhysicalSize;
  ui.Size? _currentSurfaceSize;
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

  /// Creates a <canvas> and SkSurface for the given [size].
  CkSurface createOrUpdateSurface(ui.Size size) {
    if (size.isEmpty) {
      throw CanvasKitError('Cannot create surfaces of empty size.');
    }

      // Check if the window is the same size as before, and if so, don't allocate
      // a new canvas as the previous canvas is big enough to fit everything.
      final ui.Size? previousSurfaceSize = _currentSurfaceSize;
      if (previousSurfaceSize != null &&
          size.width == previousSurfaceSize.width &&
          size.height == previousSurfaceSize.height) {
        // The existing surface is still reusable.
        if (window.devicePixelRatio != _currentDevicePixelRatio) {
          _updateLogicalHtmlCanvasSize();
          _translateCanvas();
        }
        return _surface!;
      }

      final ui.Size? previousCanvasSize = _currentCanvasPhysicalSize;
      // Initialize a new, larger, canvas. If the size is growing, then make the
      // new canvas larger than required to avoid many canvas creations.
      if (previousCanvasSize != null &&
          (size.width > previousCanvasSize.width ||
              size.height > previousCanvasSize.height)) {
        final ui.Size newSize = size * 1.4;
        _surface?.dispose();
        _surface = null;
        offscreenCanvas!.width = newSize.width;
        offscreenCanvas!.height = newSize.height;
        _currentCanvasPhysicalSize = newSize;
        _pixelWidth = newSize.width.ceil();
        _pixelHeight = newSize.height.ceil();
        _updateLogicalHtmlCanvasSize();
      }

    // This is the first frame we have rendered with this canvas.
    if (_currentCanvasPhysicalSize == null) {
      _surface?.dispose();
      _surface = null;
      _addedToScene = false;
      _grContext?.releaseResourcesAndAbandonContext();
      _grContext?.delete();
      _grContext = null;

      _createNewCanvas(size);
      _currentCanvasPhysicalSize = size;
    } else if (window.devicePixelRatio != _currentDevicePixelRatio) {
      _updateLogicalHtmlCanvasSize();
    }

    _currentDevicePixelRatio = window.devicePixelRatio;
    _currentSurfaceSize = size;
    _translateCanvas();
    _surface?.dispose();
    _surface = _createNewSurface(size);
    return _surface!;
  }

  void dispose() {
    htmlElement.remove();
  }
}
