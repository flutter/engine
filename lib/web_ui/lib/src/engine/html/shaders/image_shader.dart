// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:js_util' as js_util;
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import '../offscreen_canvas.dart';
import '../../html_image_codec.dart';

class EngineImageShader implements ui.ImageShader {
  EngineImageShader(ui.Image image, this.tileModeX, this.tileModeY, this.matrix4, this.filterQuality)
      : this.image = image as HtmlImage;

  final ui.TileMode tileModeX;
  final ui.TileMode tileModeY;
  final Float64List matrix4;
  final ui.FilterQuality? filterQuality;
  final HtmlImage image;

  /// Whether fill pattern requires transform to shift tiling offset.
  bool requiresTileOffset = false;

  Object createPaintStyle(html.CanvasRenderingContext2D context,
      ui.Rect? shaderBounds, double density) {
    /// Creates a canvas rendering context pattern based on image and tile modes.
    final ui.TileMode tileX = tileModeX;
    final ui.TileMode tileY = tileModeY;
    return context.createPattern(
        _resolveTiledImageSource(image, tileX, tileY)!,
        _tileModeToHtmlRepeatAttribute(tileX, tileY))!;
  }

  /// Converts tilemode to CSS repeat attribute.
  ///
  /// CSS and Canvas2D createPattern apis only support repeated tiles.
  /// For mirroring we create a new image with mirror builtin so both
  /// repeated and mirrored modes can be supported when applied to
  /// html element background-image or used by createPattern api.
  String _tileModeToHtmlRepeatAttribute(ui.TileMode tileModeX,
      ui.TileMode tileModeY) {
    final bool repeatX = tileModeX == ui.TileMode.repeated
        || tileModeX == ui.TileMode.mirror;
    final bool repeatY = tileModeY == ui.TileMode.repeated
        || tileModeY == ui.TileMode.mirror;
    return repeatX
        ? (repeatY ? 'repeat' : 'repeat-x')
        : (repeatY ? 'repeat-y' : 'no-repeat');
  }

  /// Tiles the image and returns as image or canvas element to be used as
  /// source for a repeated pattern.
  ///
  /// Other alternative was to create a webgl shader for the area and
  /// tile in the shader, but that will generate a much larger image footprint
  /// when the pattern is small. So we opt here for mirroring by
  /// redrawing the image 2 or 4 times into a new bitmap.
  Object? _resolveTiledImageSource(HtmlImage image,
      ui.TileMode tileX, ui.TileMode tileY) {
    final int mirrorX = tileX == ui.TileMode.mirror ? 2 : 1;
    final int mirrorY = tileY == ui.TileMode.mirror ? 2 : 1;
    /// If we have no mirror, we can use image directly as pattern.
    if (mirrorX == 1 && mirrorY == 1) {
      return image.imgElement;
    }
    /// Create a new image by mirroring.
    final int imageWidth = image.width;
    final int imageHeight = image.height;
    final int newWidth = imageWidth * mirrorX;
    final int newHeight = imageHeight * mirrorY;
    OffScreenCanvas offscreenCanvas = OffScreenCanvas(newWidth, newHeight);
    Object renderContext = offscreenCanvas.getContext2d()!;
    for (int y = 0; y < mirrorY; y++) {
      for (int x = 0; x < mirrorX; x++) {
        int flipX = x != 0 ? -1 : 1;
        int flipY = y != 0 ? -1 : 1;
        /// To draw image flipped we set translate and scale and pass
        /// negative width/height to drawImage.
        if (flipX != 1 || flipY != 1) {
          js_util.callMethod(renderContext, 'scale', <dynamic>[flipX, flipY]);
        }
        js_util.callMethod(renderContext, 'drawImage',
            <dynamic>[image.imgElement,
              x == 0 ? 0 : -2 * imageWidth, y == 0 ? 0 : -2 * imageHeight]);
        if (flipX != 1 || flipY != 1) {
          /// Restore transform. This is faster than save/restore on context.
          js_util.callMethod(renderContext, 'scale', <dynamic>[flipX, flipY]);
        }
      }
    }
    // When using OffscreenCanvas and transferToImageBitmap is supported by
    // browser create ImageBitmap otherwise use more expensive canvas
    // allocation.
    if (OffScreenCanvas.supported &&
        offscreenCanvas.transferToImageBitmapSupported) {
      return offscreenCanvas.transferToImageBitmap();
    } else {
      html.CanvasElement canvas = html.CanvasElement(width: newWidth,
          height: newHeight);
      final html.CanvasRenderingContext2D ctx = canvas.context2D;
      offscreenCanvas.transferImage(ctx);
      return canvas;
    }
  }
}
