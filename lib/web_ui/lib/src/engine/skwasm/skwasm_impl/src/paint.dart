import 'package:ui/ui.dart' as ui;
import 'raw/raw_paint.dart';

class SkwasmPaint implements ui.Paint {
  factory SkwasmPaint() {
    return SkwasmPaint._fromHandle(paintCreate());
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
      paintSetBlendMode(_handle, blendMode.index);
    }
  }

  @override
  ui.PaintingStyle get style => ui.PaintingStyle.values[paintGetPaintStyle(_handle)];

  @override
  set style(ui.PaintingStyle style) => paintSetPaintStyle(_handle, style.index);

  @override
  double get strokeWidth => paintGetStrokeWidth(_handle);

  @override
  set strokeWidth(double width) => paintSetStrokeWidth(_handle, width);

  @override
  ui.StrokeCap get strokeCap => ui.StrokeCap.values[paintGetStrokeCap(_handle)];

  @override
  set strokeCap(ui.StrokeCap cap) => paintSetStrokeCap(_handle, cap.index);

  @override
  ui.StrokeJoin get strokeJoin => ui.StrokeJoin.values[paintGetStrokeJoin(_handle)];

  @override
  set strokeJoin(ui.StrokeJoin join) => paintSetStrokeJoin(_handle, join.index);

  @override
  bool get isAntiAlias => paintGetAntiAlias(_handle);

  @override
  set isAntiAlias(bool value) => paintSetAntiAlias(_handle, value);

  @override
  ui.Color get color => ui.Color(paintGetColorInt(_handle));

  @override
  set color(ui.Color color) => paintSetColorInt(_handle, color.value);

  @override
  double get strokeMiterLimit => paintGetMiterLimit(_handle);

  @override
  set strokeMiterLimit(double limit) => paintSetMiterLimit(_handle, limit);

  // Unimplemented stuff below
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
}
