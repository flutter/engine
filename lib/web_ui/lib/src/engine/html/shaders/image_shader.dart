// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:js_util' as js_util;
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import '../offscreen_canvas.dart';
import '../render_vertices.dart';
import '../../browser_detection.dart';
import '../../html_image_codec.dart';
import '../../vector_math.dart';
import 'vertex_shaders.dart';
import 'webgl_context.dart';

class EngineImageShader implements ui.ImageShader {
  EngineImageShader(ui.Image image, this.tileModeX, this.tileModeY,
      this.matrix4, this.filterQuality)
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
    if (tileX != ui.TileMode.clamp && tileY != ui.TileMode.clamp) {
      return context.createPattern(
          _resolveTiledImageSource(image, tileX, tileY)!,
          _tileModeToHtmlRepeatAttribute(tileX, tileY))!;
    } else {
      initWebGl();
      return _createGlShader(context, shaderBounds, density);
    }
  }

  /// Converts tilemode to CSS repeat attribute.
  ///
  /// CSS and Canvas2D createPattern apis only support repeated tiles.
  /// For mirroring we create a new image with mirror builtin so both
  /// repeated and mirrored modes can be supported when applied to
  /// html element background-image or used by createPattern api.
  String _tileModeToHtmlRepeatAttribute(
      ui.TileMode tileModeX, ui.TileMode tileModeY) {
    final bool repeatX =
        tileModeX == ui.TileMode.repeated || tileModeX == ui.TileMode.mirror;
    final bool repeatY =
        tileModeY == ui.TileMode.repeated || tileModeY == ui.TileMode.mirror;
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
  Object? _resolveTiledImageSource(
      HtmlImage image, ui.TileMode tileX, ui.TileMode tileY) {
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
        js_util.callMethod(renderContext, 'drawImage', <dynamic>[
          image.imgElement,
          x == 0 ? 0 : -2 * imageWidth,
          y == 0 ? 0 : -2 * imageHeight
        ]);
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
      html.CanvasElement canvas =
          html.CanvasElement(width: newWidth, height: newHeight);
      final html.CanvasRenderingContext2D ctx = canvas.context2D;
      offscreenCanvas.transferImage(ctx);
      return canvas;
    }
  }

  /// Creates an image with tiled/transformed images.
  html.CanvasPattern _createGlShader(html.CanvasRenderingContext2D? ctx,
      ui.Rect? shaderBounds, double density) {
    assert(shaderBounds != null);
    int widthInPixels = shaderBounds!.width.ceil();
    int heightInPixels = shaderBounds.height.ceil();
    assert(widthInPixels > 0 && heightInPixels > 0);

    /// Render tiles into a bitmap and create a canvas pattern.
    final bool isWebGl2 = webGLVersion == WebGLVersion.webgl2;

    final EngineImageShader? imageShader =
    paint.shader == null ? null : paint.shader as EngineImageShader;

    final String vertexShader = imageShader == null
        ? VertexShaders.writeBaseVertexShader()
        : VertexShaders.writeTextureVertexShader();
    final String fragmentShader = imageShader == null
        ? _writeVerticesFragmentShader()
        : FragmentShaders.writeTextureFragmentShader(
        isWebGl2, imageShader.tileModeX, imageShader.tileModeY);

    GlContext gl =
    GlContextCache.createGlContext(widthInPixels, heightInPixels)!;

    GlProgram glProgram = gl.cacheProgram(vertexShader, fragmentShader);
    gl.useProgram(glProgram);

    Object? positionAttributeLocation =
    gl.getAttributeLocation(glProgram.program, 'position');

    setupVertexTransforms(gl, glProgram, offsetX, offsetY,
        widthInPixels.toDouble(), heightInPixels.toDouble(), transform);

    if (imageShader != null) {
      /// To map from vertex position to texture coordinate in 0..1 range,
      /// we setup scalar to be used in vertex shader.
      setupTextureScalar(
          gl,
          glProgram,
          1.0 / imageShader.image.width.toDouble(),
          1.0 / imageShader.image.height.toDouble());
    }

    final Float32List vertices = Float32List(8);
    vertices[0] = left;
    vertices[1] = top;
    vertices[2] = right;
    vertices[3] = top;
    vertices[4] = right;
    vertices[5] = bottom;
    vertices[6] = left;
    vertices[7] = bottom;

    // Setup geometry.
    Object positionsBuffer = gl.createBuffer()!;
    assert(positionsBuffer != null); // ignore: unnecessary_null_comparison
    gl.bindArrayBuffer(positionsBuffer);
    gl.bufferData(vertices, gl.kStaticDraw);
    // Point an attribute to the currently bound vertex buffer object.
    js_util.callMethod(gl.glContext, 'vertexAttribPointer',
        <dynamic>[0, 2, gl.kFloat, false, 0, 0]);
    gl.enableVertexAttribArray(0);

    gl.clear();
    gl.viewport(0, 0, widthInPixels.toDouble(), heightInPixels.toDouble());
    gl.drawElements(
        gl.kTriangles, VertexShaders.vertexIndicesForRect.length, gl.kUnsignedShort);

    Object? image = gl.readPatternData();
    gl.bindArrayBuffer(null);
    gl.bindElementArrayBuffer(null);
    return ctx!.createPattern(image!, 'no-repeat')!;
  }
}
