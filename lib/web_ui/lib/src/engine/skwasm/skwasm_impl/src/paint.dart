import 'dart:wasm';

import 'package:ui/ui.dart' as ui;
import 'raw/raw_paint.dart';

class SkwasmPaint implements ui.Paint {
  factory SkwasmPaint() {
    return SkwasmPaint._fromHandle(paint_create());
  }

  SkwasmPaint._fromHandle(this._handle);
  PaintHandle _handle;

  PaintHandle get handle => _handle;

  ui.BlendMode _cachedBlendMode = ui.BlendMode.srcOver;

  @override
  ui.BlendMode get blendMode {
    return _cachedBlendMode;
  }

  @override
  set blendMode(ui.BlendMode blendMode) {
    if (_cachedBlendMode != blendMode) {
      _cachedBlendMode = blendMode;
      paint_setBlendMode(_handle, blendMode.index.toWasmI32());
    }
  }

  @override
  ui.PaintingStyle get style {
    return ui.PaintingStyle.values[paint_getPaintStyle(_handle).toIntSigned()];
  }

  @override
  set style(ui.PaintingStyle style) {
    paint_setPaintStyle(_handle, style.index.toWasmI32());
  }

  @override
  double get strokeWidth {
    return paint_getStrokeWidth(_handle).toDouble();
  }

  @override
  set strokeWidth(double width) {
    paint_setStrokeWidth(_handle, width.toWasmF32());
  }

  @override
  ui.StrokeCap get strokeCap {
    return ui.StrokeCap.values[paint_getStrokeCap(_handle).toIntSigned()];
  }

  @override
  set strokeCap(ui.StrokeCap cap) {
    paint_setStrokeCap(_handle, cap.index.toWasmI32());
  }

  @override
  ui.StrokeJoin get strokeJoin {
    return ui.StrokeJoin.values[paint_getStrokeJoin(_handle).toIntSigned()];
  }

  @override
  set strokeJoin(ui.StrokeJoin join) {
    paint_setStrokeJoin(_handle, join.index.toWasmI32());
  }

  @override
  bool get isAntiAlias {
    return paint_getAntiAlias(_handle).toIntSigned() != 0;
  }
  @override
  set isAntiAlias(bool value) {
    paint_setAntiAlias(_handle, value ? 1.toWasmI32() : 0.toWasmI32());
  }

  @override
  ui.Color get color {
    return ui.Color(paint_getColorInt(_handle).toIntSigned());
  }

  @override
  set color(ui.Color color) {
    paint_setColorInt(_handle, color.value.toWasmI32());
  }
  
  @override
  ui.ColorFilter? colorFilter;
  
  @override
  ui.FilterQuality filterQuality = ui.FilterQuality.none;
  
  @override
  ui.ImageFilter? imageFilter;
  
  @override
  bool invertColors = false;
  
  @override
  ui.MaskFilter? maskFilter;
  
  @override
  ui.Shader? shader;
  
  @override
  double strokeMiterLimit = 0.0;  
}
