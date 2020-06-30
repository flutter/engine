// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// Bindings for CanvasKit JavaScript API.
part of engine;

void initializeCanvasKitBindings(js.JsObject canvasKit) {
  // Because JsObject cannot be cast to a @JS type, we stash CanvasKit into
  // a global and use the [canvasKitJs] getter to access it.
  js.JsObject.fromBrowserObject(html.window)['flutter_canvas_kit'] = canvasKit;
  SkPaint().setStrokeWidth(3.0);
}

@JS('window.flutter_canvas_kit')
external CanvasKit get canvasKitJs;

@JS()
class CanvasKit {
  external SkBlendModeEnum get BlendMode;
  external SkPaintStyleEnum get PaintStyle;
  external SkStrokeCapEnum get StrokeCap;
  external SkStrokeJoinEnum get StrokeJoin;
  external SkFilterQualityEnum get FilterQuality;
  external SkTileModeEnum get TileMode;
  external SkAnimatedImage MakeAnimatedImageFromEncoded(Uint8List imageData);
  external SkShaderNamespace get SkShader;
}

@JS()
class SkStrokeCapEnum {
  external SkStrokeCap get Butt;
  external SkStrokeCap get Round;
  external SkStrokeCap get Square;
}

@JS()
class SkStrokeCap {
  external int get value;
}

final List<SkStrokeCap> _skStrokeCaps = <SkStrokeCap>[
  canvasKitJs.StrokeCap.Butt,
  canvasKitJs.StrokeCap.Round,
  canvasKitJs.StrokeCap.Square,
];

SkStrokeCap toSkStrokeCap(ui.StrokeCap strokeCap) {
  return _skStrokeCaps[strokeCap.index];
}

@JS()
class SkPaintStyleEnum {
  external SkPaintStyle get Stroke;
  external SkPaintStyle get Fill;
}

@JS()
class SkPaintStyle {
  external int get value;
}

final List<SkPaintStyle> _skPaintStyles = <SkPaintStyle>[
  canvasKitJs.PaintStyle.Fill,
  canvasKitJs.PaintStyle.Stroke,
];

SkPaintStyle toSkPaintStyle(ui.PaintingStyle paintStyle) {
  return _skPaintStyles[paintStyle.index];
}

@JS()
class SkBlendModeEnum {
  external SkBlendMode get Clear;
  external SkBlendMode get Src;
  external SkBlendMode get Dst;
  external SkBlendMode get SrcOver;
  external SkBlendMode get DstOver;
  external SkBlendMode get SrcIn;
  external SkBlendMode get DstIn;
  external SkBlendMode get SrcOut;
  external SkBlendMode get DstOut;
  external SkBlendMode get SrcATop;
  external SkBlendMode get DstATop;
  external SkBlendMode get Xor;
  external SkBlendMode get Plus;
  external SkBlendMode get Modulate;
  external SkBlendMode get Screen;
  external SkBlendMode get Overlay;
  external SkBlendMode get Darken;
  external SkBlendMode get Lighten;
  external SkBlendMode get ColorDodge;
  external SkBlendMode get ColorBurn;
  external SkBlendMode get HardLight;
  external SkBlendMode get SoftLight;
  external SkBlendMode get Difference;
  external SkBlendMode get Exclusion;
  external SkBlendMode get Multiply;
  external SkBlendMode get Hue;
  external SkBlendMode get Saturation;
  external SkBlendMode get Color;
  external SkBlendMode get Luminosity;
}

@JS()
class SkBlendMode {
  external int get value;
}

final List<SkBlendMode> _skBlendModes = <SkBlendMode>[
  canvasKitJs.BlendMode.Clear,
  canvasKitJs.BlendMode.Src,
  canvasKitJs.BlendMode.Dst,
  canvasKitJs.BlendMode.SrcOver,
  canvasKitJs.BlendMode.DstOver,
  canvasKitJs.BlendMode.SrcIn,
  canvasKitJs.BlendMode.DstIn,
  canvasKitJs.BlendMode.SrcOut,
  canvasKitJs.BlendMode.DstOut,
  canvasKitJs.BlendMode.SrcATop,
  canvasKitJs.BlendMode.DstATop,
  canvasKitJs.BlendMode.Xor,
  canvasKitJs.BlendMode.Plus,
  canvasKitJs.BlendMode.Modulate,
  canvasKitJs.BlendMode.Screen,
  canvasKitJs.BlendMode.Overlay,
  canvasKitJs.BlendMode.Darken,
  canvasKitJs.BlendMode.Lighten,
  canvasKitJs.BlendMode.ColorDodge,
  canvasKitJs.BlendMode.ColorBurn,
  canvasKitJs.BlendMode.HardLight,
  canvasKitJs.BlendMode.SoftLight,
  canvasKitJs.BlendMode.Difference,
  canvasKitJs.BlendMode.Exclusion,
  canvasKitJs.BlendMode.Multiply,
  canvasKitJs.BlendMode.Hue,
  canvasKitJs.BlendMode.Saturation,
  canvasKitJs.BlendMode.Color,
  canvasKitJs.BlendMode.Luminosity,
];

SkBlendMode toSkBlendMode(ui.BlendMode blendMode) {
  return _skBlendModes[blendMode.index];
}

@JS()
class SkStrokeJoinEnum {
  external SkStrokeJoin get Miter;
  external SkStrokeJoin get Round;
  external SkStrokeJoin get Bevel;
}

@JS()
class SkStrokeJoin {
  external int get value;
}

final List<SkStrokeJoin> _skStrokeJoins = <SkStrokeJoin>[
  canvasKitJs.StrokeJoin.Miter,
  canvasKitJs.StrokeJoin.Round,
  canvasKitJs.StrokeJoin.Bevel,
];

SkStrokeJoin toSkStrokeJoin(ui.StrokeJoin strokeJoin) {
  return _skStrokeJoins[strokeJoin.index];
}


@JS()
class SkFilterQualityEnum {
  external SkFilterQuality get None;
  external SkFilterQuality get Low;
  external SkFilterQuality get Medium;
  external SkFilterQuality get High;
}

@JS()
class SkFilterQuality {
  external int get value;
}

final List<SkFilterQuality> _skFilterQualitys = <SkFilterQuality>[
  canvasKitJs.FilterQuality.None,
  canvasKitJs.FilterQuality.Low,
  canvasKitJs.FilterQuality.Medium,
  canvasKitJs.FilterQuality.High,
];

SkFilterQuality toSkFilterQuality(ui.FilterQuality filterQuality) {
  return _skFilterQualitys[filterQuality.index];
}


@JS()
class SkTileModeEnum {
  external SkTileMode get Clamp;
  external SkTileMode get Repeat;
  external SkTileMode get Mirror;
}

@JS()
class SkTileMode {
  external int get value;
}

final List<SkTileMode> _skTileModes = <SkTileMode>[
  canvasKitJs.TileMode.Clamp,
  canvasKitJs.TileMode.Repeat,
  canvasKitJs.TileMode.Mirror,
];

SkTileMode toSkTileMode(ui.TileMode filterQuality) {
  return _skTileModes[filterQuality.index];
}

@JS()
class SkAnimatedImage {
  external int getFrameCount();
  /// Returns duration in milliseconds.
  external int getRepetitionCount();
  external int decodeNextFrame();
  external SkImage getCurrentFrame();
  external int width();
  external int height();

  /// Deletes the C++ object.
  ///
  /// This object is no longer usable after calling this method.
  external void delete();
}

@JS()
class SkImage {
  external void delete();
  external int width();
  external int height();
  external SkShader makeShader(SkTileMode tileModeX, SkTileMode tileModeY);
}

@JS()
class SkShaderNamespace {
  // TODO(yjbanov): implement other shader types.
  external SkShader MakeLinearGradient(
    List<double> from,
    List<double> to,
    List<Float32List> colors,
    List<double> colorStops,
    SkTileMode tileMode,
  );
}

@JS()
class SkShader {

}

// This needs to be bound to top-level because SkPaint is initialized
// with `new`. Also in Dart you can't write this:
//
//     external SkPaint SkPaint();
@JS('window.flutter_canvas_kit.SkPaint')
class SkPaint {
  // TODO(yjbanov): implement invertColors, see paint.cc
  external SkPaint();
  external void setBlendMode(SkBlendMode blendMode);
  external void setStyle(SkPaintStyle paintStyle);
  external void setStrokeWidth(double width);
  external void setStrokeCap(SkStrokeCap cap);
  external void setStrokeJoin(SkStrokeJoin join);
  external void setAntiAlias(bool isAntiAlias);
  external void setColorInt(int color);
  external void setShader(SkShader shader);
  external void setMaskFilter(SkMaskFilter maskFilter);
  external void setFilterQuality(SkFilterQuality filterQuality);
  external void setColorFilter(SkColorFilter colorFilter);
  external void setStrokeMiter(double miterLimit);
  external void setImageFilter(SkImageFilter imageFilter);
}

@JS()
class SkMaskFilter {
  // TODO(yjbanov): implement
}

@JS()
class SkColorFilter {
  // TODO(yjbanov): implement
}

@JS()
class SkImageFilter {
  // TODO(yjbanov): implement
}
