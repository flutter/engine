// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:js_util' as js_util;
import 'package:ui/src/engine.dart';

/// Polyfill for html.OffscreenCanvas that is not supported on some browsers.
class OffScreenCanvas {
  html.OffscreenCanvas? _canvas;
  html.CanvasElement? _glCanvas;
  int width;
  int height;
  static bool? _supported;

  OffScreenCanvas(this.width, this.height) {
    if (OffScreenCanvas.supported) {
      _canvas = html.OffscreenCanvas(width, height);
    } else {
      _glCanvas = html.CanvasElement(
        width: width,
        height: height,
      );
      _glCanvas!.className = 'gl-canvas';
      final double cssWidth = width / EnginePlatformDispatcher.browserDevicePixelRatio;
      final double cssHeight = height / EnginePlatformDispatcher.browserDevicePixelRatio;
      _glCanvas!.style
        ..position = 'absolute'
        ..width = '${cssWidth}px'
        ..height = '${cssHeight}px';
    }
  }

  void dispose() {
    _canvas = null;
    _glCanvas = null;
  }

  /// Returns CanvasRenderContext2D or OffscreenCanvasRenderingContext2D to
  /// paint into.
  Object? getContext2d() {
    return (_canvas != null
        ? _canvas!.getContext('2d')
        : _glCanvas!.getContext('2d'));
  }

  /// Feature detection for transferToImageBitmap on OffscreenCanvas.
  bool get transferToImageBitmapSupported =>
      js_util.hasProperty(_canvas!, 'transferToImageBitmap');

  /// Creates an ImageBitmap object from the most recently rendered image
  /// of the OffscreenCanvas.
  ///
  /// !Warning API still in experimental status, feature detect before using.
  Object? transferToImageBitmap() {
    return js_util.callMethod(_canvas!, 'transferToImageBitmap',
        <dynamic>[]);
  }

  /// Draws canvas contents to a rendering context.
  void transferImage(Object targetContext) {
    // Actual size of canvas may be larger than viewport size. Use
    // source/destination to draw part of the image data.
    js_util.callMethod(targetContext, 'drawImage',
        <dynamic>[_canvas ?? _glCanvas!, 0, 0, width, height,
          0, 0, width, height]);
  }

  /// Feature detects OffscreenCanvas.
  static bool get supported => _supported ??=
      js_util.hasProperty(html.window, 'OffscreenCanvas');
}
