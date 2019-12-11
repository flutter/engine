// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// The implementation of [ui.Paint] used by the CanvasKit backend.
class SkPaint extends SkiaObject implements ui.Paint {
  SkPaint();

  static const ui.Color _defaultPaintColor = ui.Color(0xFF000000);
  static final js.JsObject _skPaintStyleStroke = canvasKit['PaintStyle']['Stroke'];
  static final js.JsObject _skPaintStyleFill = canvasKit['PaintStyle']['Fill'];

  @override
  ui.BlendMode get blendMode => _blendMode;
  @override
  set blendMode(ui.BlendMode value) {
    _blendMode = value;
    _syncBlendMode();
  }
  void _syncBlendMode() {
    final js.JsObject skBlendMode = makeSkBlendMode(_blendMode);
    skiaObject.callMethod('setBlendMode', <js.JsObject>[skBlendMode]);
  }
  ui.BlendMode _blendMode = ui.BlendMode.srcOver;

  @override
  ui.PaintingStyle get style => _style;
  @override
  set style(ui.PaintingStyle value) {
    _style = value;
    _syncStyle();
  }
  void _syncStyle() {
    js.JsObject skPaintStyle;
    switch (_style) {
      case ui.PaintingStyle.stroke:
        skPaintStyle = _skPaintStyleStroke;
        break;
      case ui.PaintingStyle.fill:
        skPaintStyle = _skPaintStyleFill;
        break;
    }
    skiaObject.callMethod('setStyle', <js.JsObject>[skPaintStyle]);
  }
  ui.PaintingStyle _style = ui.PaintingStyle.fill;

  @override
  double get strokeWidth => _strokeWidth;
  @override
  set strokeWidth(double value) {
    _strokeWidth = value;
    _syncStrokeWidth();
  }
  void _syncStrokeWidth() {
    skiaObject.callMethod('setStrokeWidth', <double>[strokeWidth]);
  }
  double _strokeWidth = 0.0;

  @override
  // TODO(yjbanov): implement
  ui.StrokeCap get strokeCap => _strokeCap;
  @override
  set strokeCap(ui.StrokeCap value) {
    _strokeCap = value;
  }
  ui.StrokeCap _strokeCap = ui.StrokeCap.butt;

  @override
  // TODO(yjbanov): implement
  ui.StrokeJoin get strokeJoin => _strokeJoin;
  @override
  set strokeJoin(ui.StrokeJoin value) {
    _strokeJoin = value;
  }
  ui.StrokeJoin _strokeJoin = ui.StrokeJoin.miter;

  @override
  bool get isAntiAlias => _isAntiAlias;
  @override
  set isAntiAlias(bool value) {
    _isAntiAlias = value;
    _syncAntiAlias();
  }
  _syncAntiAlias() {
    skiaObject.callMethod('setAntiAlias', <bool>[_isAntiAlias]);
  }
  bool _isAntiAlias = true;

  @override
  ui.Color get color => _color;
  @override
  set color(ui.Color value) {
    _color = value;
    int colorValue;
    if (_color != null) {
      colorValue = _color.value;
    }
    skiaObject.callMethod('setColor', <int>[colorValue]);
  }
  ui.Color _color = _defaultPaintColor;

  @override
  // TODO(yjbanov): implement
  bool get invertColors => _invertColors;
  @override
  set invertColors(bool value) {
    _invertColors = value;
  }
  bool _invertColors = false;

  @override
  ui.Shader get shader => _shader;
  @override
  set shader(ui.Shader value) {
    _shader = value;
    js.JsObject skShader;
    if (_shader != null) {
      skShader = _shader.createSkiaShader();
    }
    skiaObject.callMethod('setShader', <js.JsObject>[skShader]);
  }
  EngineGradient _shader;

  @override
  ui.MaskFilter get maskFilter => _maskFilter;
  @override
  set maskFilter(ui.MaskFilter value) {
    _maskFilter = value;
    if (_maskFilter != null) {
      final ui.BlurStyle blurStyle = _maskFilter.webOnlyBlurStyle;
      final double sigma = _maskFilter.webOnlySigma;

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
      skiaObject.callMethod('setMaskFilter', <js.JsObject>[skMaskFilter]);
    } else {
      skiaObject.callMethod('setMaskFilter', <js.JsObject>[null]);
    }
  }
  ui.MaskFilter _maskFilter;

  @override
  // TODO(yjbanov): implement
  ui.FilterQuality get filterQuality => _filterQuality;
  @override
  set filterQuality(ui.FilterQuality value) {
    _filterQuality = value;
  }
  ui.FilterQuality _filterQuality = ui.FilterQuality.none;

  @override
  ui.ColorFilter get colorFilter => _colorFilter;
  @override
  set colorFilter(ui.ColorFilter value) {
    _colorFilter = value;
    if (colorFilter != null) {
      EngineColorFilter engineFilter = colorFilter;
      SkColorFilter skFilter = engineFilter._toSkColorFilter();
      skiaObject.callMethod('setColorFilter', <js.JsObject>[skFilter.skColorFilter]);
    }
  }
  ui.ColorFilter _colorFilter;

  @override
  // TODO(yjbanov): implement
  double get strokeMiterLimit => _strokeMiterLimit;
  @override
  set strokeMiterLimit(double value) {
    _strokeMiterLimit = value;
  }
  double _strokeMiterLimit = 0.0;

  @override
  ui.ImageFilter get imageFilter => _imageFilter;
  @override
  set imageFilter(ui.ImageFilter value) {
    _imageFilter = value;
    if (imageFilter != null) {
      final SkImageFilter skImageFilter = imageFilter;
      skiaObject.callMethod(
          'setImageFilter', <js.JsObject>[skImageFilter.skImageFilter]);
    }
  }
  ui.ImageFilter _imageFilter;

  @override
  js.JsObject createDefault() {
    final obj = js.JsObject(canvasKit['SkPaint']);
    _syncAntiAlias();
    return obj;
  }

  @override
  js.JsObject resurrect() {
    final obj = js.JsObject(canvasKit['SkPaint']);
    _syncAntiAlias();
    return obj;
  }
}
