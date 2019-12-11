// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// The implementatin of [ui.Paint] used by the CanvasKit backend.
class SkPaint implements ui.Paint {
  SkPaint();

  static const ui.Color _defaultPaintColor = ui.Color(0xFF000000);

  ui.BlendMode get blendMode => _blendMode;
  set blendMode(ui.BlendMode value) {
    _blendMode = value;
  }
  ui.BlendMode _blendMode = ui.BlendMode.srcOver;

  ui.PaintingStyle get style => _style;
  set style(ui.PaintingStyle value) {
    _style = value;
  }
  ui.PaintingStyle _style = ui.PaintingStyle.fill;

  double get strokeWidth => _strokeWidth;
  set strokeWidth(double value) {
    _strokeWidth = value;
  }
  double _strokeWidth = 0.0;

  ui.StrokeCap get strokeCap => _strokeCap;
  set strokeCap(ui.StrokeCap value) {
    _strokeCap = value;
  }
  ui.StrokeCap _strokeCap = ui.StrokeCap.butt;

  ui.StrokeJoin get strokeJoin => _strokeJoin;
  set strokeJoin(ui.StrokeJoin value) {
    _strokeJoin = value;
  }
  ui.StrokeJoin _strokeJoin = ui.StrokeJoin.miter;

  bool get isAntiAlias => _isAntiAlias;
  set isAntiAlias(bool value) {
    _isAntiAlias = value;
  }
  bool _isAntiAlias = true;

  ui.Color get color => _color;
  set color(ui.Color value) {
    _color = value;
  }
  ui.Color _color = _defaultPaintColor;

  bool get invertColors => _invertColors;
  set invertColors(bool value) {
    _invertColors = value;
  }
  bool _invertColors = false;

  ui.Shader get shader => _shader;
  set shader(ui.Shader value) {
    _shader = value;
  }
  ui.Shader _shader;

  ui.MaskFilter get maskFilter => _maskFilter;
  set maskFilter(ui.MaskFilter value) {
    _maskFilter = value;
  }
  ui.MaskFilter _maskFilter;

  ui.FilterQuality get filterQuality => _filterQuality;
  set filterQuality(ui.FilterQuality value) {
    _filterQuality = value;
  }
  ui.FilterQuality _filterQuality = ui.FilterQuality.none;

  ui.ColorFilter get colorFilter => _colorFilter;
  set colorFilter(ui.ColorFilter value) {
    _colorFilter = value;
  }
  ui.ColorFilter _colorFilter;

  double get strokeMiterLimit => _strokeMiterLimit;
  set strokeMiterLimit(double value) {
    _strokeMiterLimit = value;
  }
  double _strokeMiterLimit = 0.0;

  ui.ImageFilter get imageFilter => _imageFilter;
  set imageFilter(ui.ImageFilter value) {
    _imageFilter = value;
  }
  ui.ImageFilter _imageFilter;

  js.JsObject makeSkPaint() {
    final js.JsObject skPaint = js.JsObject(canvasKit['SkPaint']);

    if (shader != null) {
      final EngineGradient engineShader = shader;
      skPaint.callMethod(
          'setShader', <js.JsObject>[engineShader.createSkiaShader()]);
    }

    if (color != null) {
      skPaint.callMethod('setColor', <int>[color.value]);
    }

    js.JsObject skPaintStyle;
    switch (style) {
      case ui.PaintingStyle.stroke:
        skPaintStyle = canvasKit['PaintStyle']['Stroke'];
        break;
      case ui.PaintingStyle.fill:
        skPaintStyle = canvasKit['PaintStyle']['Fill'];
        break;
    }
    skPaint.callMethod('setStyle', <js.JsObject>[skPaintStyle]);

    js.JsObject skBlendMode = makeSkBlendMode(blendMode);
    if (skBlendMode != null) {
      skPaint.callMethod('setBlendMode', <js.JsObject>[skBlendMode]);
    }

    skPaint.callMethod('setAntiAlias', <bool>[isAntiAlias]);

    if (strokeWidth != 0.0) {
      skPaint.callMethod('setStrokeWidth', <double>[strokeWidth]);
    }

    if (maskFilter != null) {
      final ui.BlurStyle blurStyle = maskFilter.webOnlyBlurStyle;
      final double sigma = maskFilter.webOnlySigma;

      js.JsObject skBlurStyle;
      switch (blurStyle) {
        case ui.BlurStyle.normal:
          skBlurStyle = canvasKit['BlurStyle']['Normal'];
          break;
        case ui.BlurStyle.solid:
          skBlurStyle = canvasKit['BlurStyle']['Solid'];
          break;
        case ui.BlurStyle.outer:
          skBlurStyle = canvasKit['BlurStyle']['Outer'];
          break;
        case ui.BlurStyle.inner:
          skBlurStyle = canvasKit['BlurStyle']['Inner'];
          break;
      }

      final js.JsObject skMaskFilter = canvasKit
          .callMethod('MakeBlurMaskFilter', <dynamic>[skBlurStyle, sigma, true]);
      skPaint.callMethod('setMaskFilter', <js.JsObject>[skMaskFilter]);
    }

    if (imageFilter != null) {
      final SkImageFilter skImageFilter = imageFilter;
      skPaint.callMethod(
          'setImageFilter', <js.JsObject>[skImageFilter.skImageFilter]);
    }

    if (colorFilter != null) {
      EngineColorFilter engineFilter = colorFilter;
      SkColorFilter skFilter = engineFilter._toSkColorFilter();
      skPaint.callMethod('setColorFilter', <js.JsObject>[skFilter.skColorFilter]);
    }

    return skPaint;
  }
}
