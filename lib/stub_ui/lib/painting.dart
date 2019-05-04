// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

// Some methods in this file assert that their arguments are not null. These
// asserts are just to improve the error messages; they should only cover
// arguments that are either dereferenced _in Dart_, before being passed to the
// engine, or that the engine explicitly null-checks itself (after attempting to
// convert the argument to a native type). It should not be possible for a null
// or invalid value to be used by the engine even in release mode, since that
// would cause a crash. It is, however, acceptable for error messages to be much
// less useful or correct in release mode than in debug mode.
//
// Painting APIs will also warn about arguments representing NaN coordinates,
// which can not be rendered by Skia.

// Update this list when changing the list of supported codecs.
/// Stub implementation. See docs in `../ui/`.

bool _rectIsValid(Rect rect) {
  assert(rect != null, 'Rect argument was null.');
  assert(!rect.hasNaN, 'Rect argument contained a NaN value.');
  return true;
}

bool _rrectIsValid(RRect rrect) {
  assert(rrect != null, 'RRect argument was null.');
  assert(!rrect.hasNaN, 'RRect argument contained a NaN value.');
  return true;
}

bool _offsetIsValid(Offset offset) {
  assert(offset != null, 'Offset argument was null.');
  assert(!offset.dx.isNaN && !offset.dy.isNaN, 'Offset argument contained a NaN value.');
  return true;
}

bool _matrix4IsValid(Float64List matrix4) {
  assert(matrix4 != null, 'Matrix4 argument was null.');
  assert(matrix4.length == 16, 'Matrix4 must have 16 entries.');
  return true;
}

bool _radiusIsValid(Radius radius) {
  assert(radius != null, 'Radius argument was null.');
  assert(!radius.x.isNaN && !radius.y.isNaN, 'Radius argument contained a NaN value.');
  return true;
}

Color _scaleAlpha(Color a, double factor) {
  return a.withAlpha((a.alpha * factor).round().clamp(0, 255));
}

/// Stub implementation. See docs in `../ui/`.
class Color {
  /// Stub implementation. See docs in `../ui/`.
  @pragma('vm:entry-point')
  const Color(int value) : value = value & 0xFFFFFFFF;

  /// Stub implementation. See docs in `../ui/`.
  const Color.fromARGB(int a, int r, int g, int b) :
    value = (((a & 0xff) << 24) |
             ((r & 0xff) << 16) |
             ((g & 0xff) << 8)  |
             ((b & 0xff) << 0)) & 0xFFFFFFFF;

  /// Stub implementation. See docs in `../ui/`.
  const Color.fromRGBO(int r, int g, int b, double opacity) :
    value = ((((opacity * 0xff ~/ 1) & 0xff) << 24) |
              ((r                    & 0xff) << 16) |
              ((g                    & 0xff) << 8)  |
              ((b                    & 0xff) << 0)) & 0xFFFFFFFF;

  /// Stub implementation. See docs in `../ui/`.
  final int value;

  /// Stub implementation. See docs in `../ui/`.
  int get alpha => (0xff000000 & value) >> 24;

  /// Stub implementation. See docs in `../ui/`.
  double get opacity => alpha / 0xFF;

  /// Stub implementation. See docs in `../ui/`.
  int get red => (0x00ff0000 & value) >> 16;

  /// Stub implementation. See docs in `../ui/`.
  int get green => (0x0000ff00 & value) >> 8;

  /// Stub implementation. See docs in `../ui/`.
  int get blue => (0x000000ff & value) >> 0;

  /// Stub implementation. See docs in `../ui/`.
  Color withAlpha(int a) {
    return new Color.fromARGB(a, red, green, blue);
  }

  /// Stub implementation. See docs in `../ui/`.
  Color withOpacity(double opacity) {
    assert(opacity >= 0.0 && opacity <= 1.0);
    return withAlpha((255.0 * opacity).round());
  }

  /// Stub implementation. See docs in `../ui/`.
  Color withRed(int r) {
    return new Color.fromARGB(alpha, r, green, blue);
  }

  /// Stub implementation. See docs in `../ui/`.
  Color withGreen(int g) {
    return new Color.fromARGB(alpha, red, g, blue);
  }

  /// Stub implementation. See docs in `../ui/`.
  Color withBlue(int b) {
    return new Color.fromARGB(alpha, red, green, b);
  }

  // See <https://www.w3.org/TR/WCAG20/#relativeluminancedef>
  static double _linearizeColorComponent(double component) {
    if (component <= 0.03928)
      return component / 12.92;
    return math.pow((component + 0.055) / 1.055, 2.4);
  }

  /// Stub implementation. See docs in `../ui/`.
  double computeLuminance() {
    // See <https://www.w3.org/TR/WCAG20/#relativeluminancedef>
    final double R = _linearizeColorComponent(red / 0xFF);
    final double G = _linearizeColorComponent(green / 0xFF);
    final double B = _linearizeColorComponent(blue / 0xFF);
    return 0.2126 * R + 0.7152 * G + 0.0722 * B;
  }

  /// Stub implementation. See docs in `../ui/`.
  static Color lerp(Color a, Color b, double t) {
    assert(t != null);
    if (a == null && b == null)
      return null;
    if (a == null)
      return _scaleAlpha(b, t);
    if (b == null)
      return _scaleAlpha(a, 1.0 - t);
    return new Color.fromARGB(
      lerpDouble(a.alpha, b.alpha, t).toInt().clamp(0, 255),
      lerpDouble(a.red, b.red, t).toInt().clamp(0, 255),
      lerpDouble(a.green, b.green, t).toInt().clamp(0, 255),
      lerpDouble(a.blue, b.blue, t).toInt().clamp(0, 255),
    );
  }

  /// Stub implementation. See docs in `../ui/`.
  static Color alphaBlend(Color foreground, Color background) {
    final int alpha = foreground.alpha;
    if (alpha == 0x00) { // Foreground completely transparent.
      return background;
    }
    final int invAlpha = 0xff - alpha;
    int backAlpha = background.alpha;
    if (backAlpha == 0xff) { // Opaque background case
      return new Color.fromARGB(
        0xff,
        (alpha * foreground.red + invAlpha * background.red) ~/ 0xff,
        (alpha * foreground.green + invAlpha * background.green) ~/ 0xff,
        (alpha * foreground.blue + invAlpha * background.blue) ~/ 0xff,
      );
    } else { // General case
      backAlpha = (backAlpha * invAlpha) ~/ 0xff;
      final int outAlpha = alpha + backAlpha;
      assert(outAlpha != 0x00);
      return new Color.fromARGB(
        outAlpha,
        (foreground.red * alpha + background.red * backAlpha) ~/ outAlpha,
        (foreground.green * alpha + background.green * backAlpha) ~/ outAlpha,
        (foreground.blue * alpha + background.blue * backAlpha) ~/ outAlpha,
      );
    }
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other.runtimeType != runtimeType)
      return false;
    final Color typedOther = other;
    return value == typedOther.value;
  }

  @override
  int get hashCode => value.hashCode;

  @override
  String toString() => 'Color(0x${value.toRadixString(16).padLeft(8, '0')})';
}

/// Stub implementation. See docs in `../ui/`.
enum BlendMode {
  // This list comes from Skia's SkXfermode.h and the values (order) should be
  // kept in sync.
  // See: https://skia.org/user/api/skpaint#SkXfermode

  /// Stub implementation. See docs in `../ui/`.
  clear,

  /// Stub implementation. See docs in `../ui/`.
  src,

  /// Stub implementation. See docs in `../ui/`.
  dst,

  /// Stub implementation. See docs in `../ui/`.
  srcOver,

  /// Stub implementation. See docs in `../ui/`.
  dstOver,

  /// Stub implementation. See docs in `../ui/`.
  srcIn,

  /// Stub implementation. See docs in `../ui/`.
  dstIn,

  /// Stub implementation. See docs in `../ui/`.
  srcOut,

  /// Stub implementation. See docs in `../ui/`.
  dstOut,

  /// Stub implementation. See docs in `../ui/`.
  srcATop,

  /// Stub implementation. See docs in `../ui/`.
  dstATop,

  /// Stub implementation. See docs in `../ui/`.
  xor,

  /// Stub implementation. See docs in `../ui/`.
  plus,

  /// Stub implementation. See docs in `../ui/`.
  modulate,

  // Following blend modes are defined in the CSS Compositing standard.

  /// Stub implementation. See docs in `../ui/`.
  screen,  // The last coeff mode.

  /// Stub implementation. See docs in `../ui/`.
  overlay,

  /// Stub implementation. See docs in `../ui/`.
  darken,

  /// Stub implementation. See docs in `../ui/`.
  lighten,

  /// Stub implementation. See docs in `../ui/`.
  colorDodge,

  /// Stub implementation. See docs in `../ui/`.
  colorBurn,

  /// Stub implementation. See docs in `../ui/`.
  hardLight,

  /// Stub implementation. See docs in `../ui/`.
  softLight,

  /// Stub implementation. See docs in `../ui/`.
  difference,

  /// Stub implementation. See docs in `../ui/`.
  exclusion,

  /// Stub implementation. See docs in `../ui/`.
  multiply,  // The last separable mode.

  /// Stub implementation. See docs in `../ui/`.
  hue,

  /// Stub implementation. See docs in `../ui/`.
  saturation,

  /// Stub implementation. See docs in `../ui/`.
  color,

  /// Stub implementation. See docs in `../ui/`.
  luminosity,
}

/// Stub implementation. See docs in `../ui/`.
enum FilterQuality {
  // This list comes from Skia's SkFilterQuality.h and the values (order) should
  // be kept in sync.

  /// Stub implementation. See docs in `../ui/`.
  none,

  /// Stub implementation. See docs in `../ui/`.
  low,

  /// Stub implementation. See docs in `../ui/`.
  medium,

  /// Stub implementation. See docs in `../ui/`.
  high,
}

/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkPaint::Cap.
enum StrokeCap {
  /// Stub implementation. See docs in `../ui/`.
  butt,

  /// Stub implementation. See docs in `../ui/`.
  round,

  /// Stub implementation. See docs in `../ui/`.
  square,
}

/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkPaint::Join.
enum StrokeJoin {
  /// Stub implementation. See docs in `../ui/`.
  miter,

  /// Stub implementation. See docs in `../ui/`.
  round,

  /// Stub implementation. See docs in `../ui/`.
  bevel,
}

/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkPaint::Style.
enum PaintingStyle {
  // This list comes from Skia's SkPaint.h and the values (order) should be kept
  // in sync.

  /// Stub implementation. See docs in `../ui/`.
  fill,

  /// Stub implementation. See docs in `../ui/`.
  stroke,
}


/// Stub implementation. See docs in `../ui/`.
enum Clip {
  /// Stub implementation. See docs in `../ui/`.
  none,

  /// Stub implementation. See docs in `../ui/`.
  hardEdge,

  /// Stub implementation. See docs in `../ui/`.
  antiAlias,

  /// Stub implementation. See docs in `../ui/`.
  antiAliasWithSaveLayer,
}

// If we actually run on big endian machines, we'll need to do something smarter
// here. We don't use [Endian.Host] because it's not a compile-time
// constant and can't propagate into the set/get calls.
const Endian _kFakeHostEndian = Endian.little;

/// Stub implementation. See docs in `../ui/`.
class Paint {
  // Paint objects are encoded in two buffers:
  //
  // * _data is binary data in four-byte fields, each of which is either a
  //   uint32_t or a float. The default value for each field is encoded as
  //   zero to make initialization trivial. Most values already have a default
  //   value of zero, but some, such as color, have a non-zero default value.
  //   To encode or decode these values, XOR the value with the default value.
  //
  // * _objects is a list of unencodable objects, typically wrappers for native
  //   objects. The objects are simply stored in the list without any additional
  //   encoding.
  //
  // The binary format must match the deserialization code in paint.cc.

  final ByteData _data = new ByteData(_kDataByteCount);
  static const int _kIsAntiAliasIndex = 0;
  static const int _kColorIndex = 1;
  static const int _kBlendModeIndex = 2;
  static const int _kStyleIndex = 3;
  static const int _kStrokeWidthIndex = 4;
  static const int _kStrokeCapIndex = 5;
  static const int _kStrokeJoinIndex = 6;
  static const int _kStrokeMiterLimitIndex = 7;
  static const int _kFilterQualityIndex = 8;
  static const int _kColorFilterIndex = 9;
  static const int _kColorFilterColorIndex = 10;
  static const int _kColorFilterBlendModeIndex = 11;
  static const int _kMaskFilterIndex = 12;
  static const int _kMaskFilterBlurStyleIndex = 13;
  static const int _kMaskFilterSigmaIndex = 14;
  static const int _kInvertColorIndex = 15;

  static const int _kIsAntiAliasOffset = _kIsAntiAliasIndex << 2;
  static const int _kColorOffset = _kColorIndex << 2;
  static const int _kBlendModeOffset = _kBlendModeIndex << 2;
  static const int _kStyleOffset = _kStyleIndex << 2;
  static const int _kStrokeWidthOffset = _kStrokeWidthIndex << 2;
  static const int _kStrokeCapOffset = _kStrokeCapIndex << 2;
  static const int _kStrokeJoinOffset = _kStrokeJoinIndex << 2;
  static const int _kStrokeMiterLimitOffset = _kStrokeMiterLimitIndex << 2;
  static const int _kFilterQualityOffset = _kFilterQualityIndex << 2;
  static const int _kColorFilterOffset = _kColorFilterIndex << 2;
  static const int _kColorFilterColorOffset = _kColorFilterColorIndex << 2;
  static const int _kColorFilterBlendModeOffset = _kColorFilterBlendModeIndex << 2;
  static const int _kMaskFilterOffset = _kMaskFilterIndex << 2;
  static const int _kMaskFilterBlurStyleOffset = _kMaskFilterBlurStyleIndex << 2;
  static const int _kMaskFilterSigmaOffset = _kMaskFilterSigmaIndex << 2;
  static const int _kInvertColorOffset = _kInvertColorIndex << 2;
  // If you add more fields, remember to update _kDataByteCount.
  static const int _kDataByteCount = 75;

  // Binary format must match the deserialization code in paint.cc.
  List<dynamic> _objects;
  static const int _kShaderIndex = 0;
  static const int _kColorFilterMatrixIndex = 1;
  static const int _kObjectCount = 2; // Must be one larger than the largest index.

  /// Stub implementation. See docs in `../ui/`.
  bool get isAntiAlias {
    return _data.getInt32(_kIsAntiAliasOffset, _kFakeHostEndian) == 0;
  }
  set isAntiAlias(bool value) {
    // We encode true as zero and false as one because the default value, which
    // we always encode as zero, is true.
    final int encoded = value ? 0 : 1;
    _data.setInt32(_kIsAntiAliasOffset, encoded, _kFakeHostEndian);
  }

  // Must be kept in sync with the default in paint.cc.
  static const int _kColorDefault = 0xFF000000;

  /// Stub implementation. See docs in `../ui/`.
  Color get color {
    final int encoded = _data.getInt32(_kColorOffset, _kFakeHostEndian);
    return new Color(encoded ^ _kColorDefault);
  }
  set color(Color value) {
    assert(value != null);
    final int encoded = value.value ^ _kColorDefault;
    _data.setInt32(_kColorOffset, encoded, _kFakeHostEndian);
  }

  // Must be kept in sync with the default in paint.cc.
  static final int _kBlendModeDefault = BlendMode.srcOver.index;

  /// Stub implementation. See docs in `../ui/`.
  BlendMode get blendMode {
    final int encoded = _data.getInt32(_kBlendModeOffset, _kFakeHostEndian);
    return BlendMode.values[encoded ^ _kBlendModeDefault];
  }
  set blendMode(BlendMode value) {
    assert(value != null);
    final int encoded = value.index ^ _kBlendModeDefault;
    _data.setInt32(_kBlendModeOffset, encoded, _kFakeHostEndian);
  }

  /// Stub implementation. See docs in `../ui/`.
  PaintingStyle get style {
    return PaintingStyle.values[_data.getInt32(_kStyleOffset, _kFakeHostEndian)];
  }
  set style(PaintingStyle value) {
    assert(value != null);
    final int encoded = value.index;
    _data.setInt32(_kStyleOffset, encoded, _kFakeHostEndian);
  }

  /// Stub implementation. See docs in `../ui/`.
  double get strokeWidth {
    return _data.getFloat32(_kStrokeWidthOffset, _kFakeHostEndian);
  }
  set strokeWidth(double value) {
    assert(value != null);
    final double encoded = value;
    _data.setFloat32(_kStrokeWidthOffset, encoded, _kFakeHostEndian);
  }

  /// Stub implementation. See docs in `../ui/`.
  StrokeCap get strokeCap {
    return StrokeCap.values[_data.getInt32(_kStrokeCapOffset, _kFakeHostEndian)];
  }
  set strokeCap(StrokeCap value) {
    assert(value != null);
    final int encoded = value.index;
    _data.setInt32(_kStrokeCapOffset, encoded, _kFakeHostEndian);
  }

  /// Stub implementation. See docs in `../ui/`.
  StrokeJoin get strokeJoin {
    return StrokeJoin.values[_data.getInt32(_kStrokeJoinOffset, _kFakeHostEndian)];
  }
  set strokeJoin(StrokeJoin value) {
    assert(value != null);
    final int encoded = value.index;
    _data.setInt32(_kStrokeJoinOffset, encoded, _kFakeHostEndian);
  }

  // Must be kept in sync with the default in paint.cc.
  static const double _kStrokeMiterLimitDefault = 4.0;

  /// Stub implementation. See docs in `../ui/`.
  double get strokeMiterLimit {
    return _data.getFloat32(_kStrokeMiterLimitOffset, _kFakeHostEndian);
  }
  set strokeMiterLimit(double value) {
    assert(value != null);
    final double encoded = value - _kStrokeMiterLimitDefault;
    _data.setFloat32(_kStrokeMiterLimitOffset, encoded, _kFakeHostEndian);
  }

  /// Stub implementation. See docs in `../ui/`.
  MaskFilter get maskFilter {
    switch (_data.getInt32(_kMaskFilterOffset, _kFakeHostEndian)) {
      case MaskFilter._TypeNone:
        return null;
      case MaskFilter._TypeBlur:
        return new MaskFilter.blur(
          BlurStyle.values[_data.getInt32(_kMaskFilterBlurStyleOffset, _kFakeHostEndian)],
          _data.getFloat32(_kMaskFilterSigmaOffset, _kFakeHostEndian),
        );
    }
    return null;
  }
  set maskFilter(MaskFilter value) {
    if (value == null) {
      _data.setInt32(_kMaskFilterOffset, MaskFilter._TypeNone, _kFakeHostEndian);
      _data.setInt32(_kMaskFilterBlurStyleOffset, 0, _kFakeHostEndian);
      _data.setFloat32(_kMaskFilterSigmaOffset, 0.0, _kFakeHostEndian);
    } else {
      // For now we only support one kind of MaskFilter, so we don't need to
      // check what the type is if it's not null.
      _data.setInt32(_kMaskFilterOffset, MaskFilter._TypeBlur, _kFakeHostEndian);
      _data.setInt32(_kMaskFilterBlurStyleOffset, value._style.index, _kFakeHostEndian);
      _data.setFloat32(_kMaskFilterSigmaOffset, value._sigma, _kFakeHostEndian);
    }
  }

  /// Stub implementation. See docs in `../ui/`.
  // TODO(ianh): verify that the image drawing methods actually respect this
  FilterQuality get filterQuality {
    return FilterQuality.values[_data.getInt32(_kFilterQualityOffset, _kFakeHostEndian)];
  }
  set filterQuality(FilterQuality value) {
    assert(value != null);
    final int encoded = value.index;
    _data.setInt32(_kFilterQualityOffset, encoded, _kFakeHostEndian);
  }

  /// Stub implementation. See docs in `../ui/`.
  Shader get shader {
    if (_objects == null)
      return null;
    return _objects[_kShaderIndex];
  }
  set shader(Shader value) {
    _objects ??= new List<dynamic>(_kObjectCount);
    _objects[_kShaderIndex] = value;
  }

  /// Stub implementation. See docs in `../ui/`.
  ColorFilter get colorFilter {
    switch (_data.getInt32(_kColorFilterOffset, _kFakeHostEndian)) {
      case ColorFilter._TypeNone:
        return null;
      case ColorFilter._TypeMode:
        return new ColorFilter.mode(
          new Color(_data.getInt32(_kColorFilterColorOffset, _kFakeHostEndian)),
          BlendMode.values[_data.getInt32(_kColorFilterBlendModeOffset, _kFakeHostEndian)],
        );
      case ColorFilter._TypeMatrix:
        return new ColorFilter.matrix(_objects[_kColorFilterMatrixIndex]);
      case ColorFilter._TypeLinearToSrgbGamma:
        return const ColorFilter.linearToSrgbGamma();
      case ColorFilter._TypeSrgbToLinearGamma:
        return const ColorFilter.srgbToLinearGamma();
    }

    return null;
  }

  set colorFilter(ColorFilter value) {
    if (value == null) {
      _data.setInt32(_kColorFilterOffset, ColorFilter._TypeNone, _kFakeHostEndian);
      _data.setInt32(_kColorFilterColorOffset, 0, _kFakeHostEndian);
      _data.setInt32(_kColorFilterBlendModeOffset, 0, _kFakeHostEndian);

      if (_objects != null) {
        _objects[_kColorFilterMatrixIndex] = null;
      }
    } else {
      _data.setInt32(_kColorFilterOffset, value._type, _kFakeHostEndian);

      if (value._type == ColorFilter._TypeMode) {
        assert(value._color != null);
        assert(value._blendMode != null);

        _data.setInt32(_kColorFilterColorOffset, value._color.value, _kFakeHostEndian);
        _data.setInt32(_kColorFilterBlendModeOffset, value._blendMode.index, _kFakeHostEndian);
      } else if (value._type == ColorFilter._TypeMatrix) {
        assert(value._matrix != null);

        _objects ??= new List<dynamic>(_kObjectCount);
        _objects[_kColorFilterMatrixIndex] = Float32List.fromList(value._matrix);
      }
    }
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get invertColors {
    return _data.getInt32(_kInvertColorOffset, _kFakeHostEndian) == 1;
  }
  set invertColors(bool value) {
    _data.setInt32(_kInvertColorOffset, value ? 1 : 0, _kFakeHostEndian);
  }

  @override
  String toString() {
    final StringBuffer result = new StringBuffer();
    String semicolon = '';
    result.write('Paint(');
    if (style == PaintingStyle.stroke) {
      result.write('$style');
      if (strokeWidth != 0.0)
        result.write(' ${strokeWidth.toStringAsFixed(1)}');
      else
        result.write(' hairline');
      if (strokeCap != StrokeCap.butt)
        result.write(' $strokeCap');
      if (strokeJoin == StrokeJoin.miter) {
        if (strokeMiterLimit != _kStrokeMiterLimitDefault)
          result.write(' $strokeJoin up to ${strokeMiterLimit.toStringAsFixed(1)}');
      } else {
        result.write(' $strokeJoin');
      }
      semicolon = '; ';
    }
    if (isAntiAlias != true) {
      result.write('${semicolon}antialias off');
      semicolon = '; ';
    }
    if (color != const Color(_kColorDefault)) {
      if (color != null)
        result.write('$semicolon$color');
      else
        result.write('${semicolon}no color');
      semicolon = '; ';
    }
    if (blendMode.index != _kBlendModeDefault) {
      result.write('$semicolon$blendMode');
      semicolon = '; ';
    }
    if (colorFilter != null) {
      result.write('${semicolon}colorFilter: $colorFilter');
      semicolon = '; ';
    }
    if (maskFilter != null) {
      result.write('${semicolon}maskFilter: $maskFilter');
      semicolon = '; ';
    }
    if (filterQuality != FilterQuality.none) {
      result.write('${semicolon}filterQuality: $filterQuality');
      semicolon = '; ';
    }
    if (shader != null) {
      result.write('${semicolon}shader: $shader');
      semicolon = '; ';
    }
    if (invertColors)
      result.write('${semicolon}invert: $invertColors');
    result.write(')');
    return result.toString();
  }
}

/// Stub implementation. See docs in `../ui/`.
enum ImageByteFormat {
  /// Stub implementation. See docs in `../ui/`.
  rawRgba,

  /// Stub implementation. See docs in `../ui/`.
  rawUnmodified,

  /// Stub implementation. See docs in `../ui/`.
  png,
}

/// Stub implementation. See docs in `../ui/`.
enum PixelFormat {
  /// Stub implementation. See docs in `../ui/`.
  rgba8888,

  /// Stub implementation. See docs in `../ui/`.
  bgra8888,
}

/// Stub implementation. See docs in `../ui/`.
class Image {
  // This class is created by the engine, and should not be instantiated
  // or extended directly.
  //
  // To obtain an [Image] object, use [instantiateImageCodec].
  Image._();

  /// Stub implementation. See docs in `../ui/`.
  int get width {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  int get height {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Future<ByteData> toByteData({ImageByteFormat format: ImageByteFormat.rawRgba}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void dispose() {
    throw UnimplementedError();
  }

  @override
  String toString() => '[$width\u00D7$height]';
}

/// Stub implementation. See docs in `../ui/`.
typedef ImageDecoderCallback = void Function(Image result);

/// Stub implementation. See docs in `../ui/`.
class FrameInfo {
  /// Stub implementation. See docs in `../ui/`.
  FrameInfo._();

  /// Stub implementation. See docs in `../ui/`.
  Duration get duration {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Image get image {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class Codec {
  //
  // This class is created by the engine, and should not be instantiated
  // or extended directly.
  //
  // To obtain an instance of the [Codec] interface, see
  // [instantiateImageCodec].
  Codec._();

  /// Stub implementation. See docs in `../ui/`.
  int get frameCount {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  int get repetitionCount {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Future<FrameInfo> getNextFrame() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void dispose() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
Future<Codec> instantiateImageCodec(Uint8List list, {
  double decodedCacheRatioCap = 0,
}) {
  throw UnimplementedError();
}


/// Stub implementation. See docs in `../ui/`.
void decodeImageFromList(Uint8List list, ImageDecoderCallback callback) {
  throw UnimplementedError();
}

/// Stub implementation. See docs in `../ui/`.
void decodeImageFromPixels(
  Uint8List pixels,
  int width,
  int height,
  PixelFormat format,
  ImageDecoderCallback callback,
  {int rowBytes, double decodedCacheRatioCap = 0}
) {
  throw UnimplementedError();
}

/// Stub implementation. See docs in `../ui/`.
enum PathFillType {
  /// Stub implementation. See docs in `../ui/`.
  nonZero,

  /// Stub implementation. See docs in `../ui/`.
  evenOdd,
}

/// Stub implementation. See docs in `../ui/`.
// Must be kept in sync with SkPathOp
enum PathOperation {
  /// Stub implementation. See docs in `../ui/`.
  difference,
  /// Stub implementation. See docs in `../ui/`.
  intersect,
  /// Stub implementation. See docs in `../ui/`.
  union,
  /// Stub implementation. See docs in `../ui/`.
  xor,
  /// Stub implementation. See docs in `../ui/`.
  reverseDifference,
}

/// Stub implementation. See docs in `../ui/`.
class EngineLayer {
  /// Stub implementation. See docs in `../ui/`.
  EngineLayer._();
}

/// Stub implementation. See docs in `../ui/`.
class Path {
  /// Stub implementation. See docs in `../ui/`.
  Path() { throw UnimplementedError(); }

  /// Stub implementation. See docs in `../ui/`.
  Path.from(Path source);

  /// Stub implementation. See docs in `../ui/`.
  PathFillType get fillType {
    throw UnimplementedError();
  }
  set fillType(PathFillType value) {
    throw UnimplementedError();
  }


  /// Stub implementation. See docs in `../ui/`.
  void moveTo(double x, double y) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void relativeMoveTo(double dx, double dy) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void lineTo(double x, double y) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void relativeLineTo(double dx, double dy) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void quadraticBezierTo(double x1, double y1, double x2, double y2) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void relativeQuadraticBezierTo(double x1, double y1, double x2, double y2) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void cubicTo(double x1, double y1, double x2, double y2, double x3, double y3) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void relativeCubicTo(double x1, double y1, double x2, double y2, double x3, double y3) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void conicTo(double x1, double y1, double x2, double y2, double w) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void relativeConicTo(double x1, double y1, double x2, double y2, double w) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void arcTo(Rect rect, double startAngle, double sweepAngle, bool forceMoveTo) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void arcToPoint(Offset arcEnd, {
    Radius radius: Radius.zero,
    double rotation: 0.0,
    bool largeArc: false,
    bool clockwise: true,
    }) {
    throw UnimplementedError();
  }


  /// Stub implementation. See docs in `../ui/`.
  void relativeArcToPoint(Offset arcEndDelta, {
    Radius radius: Radius.zero,
    double rotation: 0.0,
    bool largeArc: false,
    bool clockwise: true,
    }) {
    assert(_offsetIsValid(arcEndDelta));
    assert(_radiusIsValid(radius));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addRect(Rect rect) {
    assert(_rectIsValid(rect));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addOval(Rect oval) {
    assert(_rectIsValid(oval));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addArc(Rect oval, double startAngle, double sweepAngle) {
    assert(_rectIsValid(oval));
    throw UnimplementedError();
  }


  /// Stub implementation. See docs in `../ui/`.
  void addPolygon(List<Offset> points, bool close) {
    assert(points != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addRRect(RRect rrect) {
    assert(_rrectIsValid(rrect));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addPath(Path path, Offset offset, {Float64List matrix4}) {
    assert(path != null); // path is checked on the engine side
    assert(_offsetIsValid(offset));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void extendWithPath(Path path, Offset offset, {Float64List matrix4}) {
    assert(path != null); // path is checked on the engine side
    assert(_offsetIsValid(offset));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void close() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void reset() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  bool contains(Offset point) {
    assert(_offsetIsValid(point));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Path shift(Offset offset) {
    assert(_offsetIsValid(offset));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Path transform(Float64List matrix4) {
    assert(_matrix4IsValid(matrix4));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  // see https://skia.org/user/api/SkPath_Reference#SkPath_getBounds
  Rect getBounds() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  static Path combine(PathOperation operation, Path path1, Path path2) {
    assert(path1 != null);
    assert(path2 != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  PathMetrics computeMetrics({bool forceClosed: false}) {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class Tangent {
  /// Stub implementation. See docs in `../ui/`.
  const Tangent(this.position, this.vector)
    : assert(position != null),
      assert(vector != null);

  /// Stub implementation. See docs in `../ui/`.
  factory Tangent.fromAngle(Offset position, double angle) {
    return new Tangent(position, new Offset(math.cos(angle), math.sin(angle)));
  }

  /// Stub implementation. See docs in `../ui/`.
  final Offset position;

  /// Stub implementation. See docs in `../ui/`.
  final Offset vector;

  /// Stub implementation. See docs in `../ui/`.
  // flip the sign to be consistent with [Path.arcTo]'s `sweepAngle`
  double get angle => -math.atan2(vector.dy, vector.dx);
}

/// Stub implementation. See docs in `../ui/`.
class PathMetrics extends collection.IterableBase<PathMetric> {

  @override
  Iterator<PathMetric> get iterator {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class PathMetricIterator implements Iterator<PathMetric> {

  @override
  PathMetric get current {
    throw UnimplementedError();
  }

  @override
  bool moveNext() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
// TODO(dnfield): Fix this if/when https://bugs.chromium.org/p/skia/issues/detail?id=8721 lands.
class PathMetric {
  PathMetric._(this._measure)
    : assert(_measure != null),
      length = _measure.length,
      isClosed = _measure.isClosed,
      contourIndex = _measure.currentContourIndex;

  /// Stub implementation. See docs in `../ui/`.
  final double length;

  /// Stub implementation. See docs in `../ui/`.
  final bool isClosed;

  /// Stub implementation. See docs in `../ui/`.
  final int contourIndex;

  final _PathMeasure _measure;


  /// Stub implementation. See docs in `../ui/`.
  Tangent getTangentForOffset(double distance) {
    if (contourIndex != _measure.currentContourIndex) {
      throw StateError('This method cannot be invoked once the underlying iterator has advanced.');
    }
    return _measure.getTangentForOffset(distance);
  }

  /// Stub implementation. See docs in `../ui/`.
  Path extractPath(double start, double end, {bool startWithMoveTo: true}) {
    if (contourIndex != _measure.currentContourIndex) {
      throw StateError('This method cannot be invoked once the underlying iterator has advanced.');
    }
    return _measure.extractPath(start, end, startWithMoveTo: startWithMoveTo);
  }

  @override
  String toString() => '$runtimeType{length: $length, isClosed: $isClosed, contourIndex:$contourIndex}';
}

class _PathMeasure {
  _PathMeasure(Path path, bool forceClosed) {
    currentContourIndex = -1; // PathMetricIterator will increment this the first time.
  }

  double get length {
    throw UnimplementedError();
  }

  Tangent getTangentForOffset(double distance) {
    throw UnimplementedError();
  }

  Path extractPath(double start, double end, {bool startWithMoveTo: true}) {
    throw UnimplementedError();
  }

  bool get isClosed {
    throw UnimplementedError();
  }

  int currentContourIndex;
}

/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkBlurStyle.
enum BlurStyle {
  // These mirror SkBlurStyle and must be kept in sync.

  /// Stub implementation. See docs in `../ui/`.
  normal,

  /// Stub implementation. See docs in `../ui/`.
  solid,

  /// Stub implementation. See docs in `../ui/`.
  outer,

  /// Stub implementation. See docs in `../ui/`.
  inner,
}

/// Stub implementation. See docs in `../ui/`.
class MaskFilter {
  /// Stub implementation. See docs in `../ui/`.
  const MaskFilter.blur(
    this._style,
    this._sigma,
  ) : assert(_style != null),
      assert(_sigma != null);

  final BlurStyle _style;
  final double _sigma;

  // The type of MaskFilter class to create for Skia.
  // These constants must be kept in sync with MaskFilterType in paint.cc.
  static const int _TypeNone = 0; // null
  static const int _TypeBlur = 1; // SkBlurMaskFilter

  @override
  bool operator ==(dynamic other) {
    if (other is! MaskFilter)
      return false;
    final MaskFilter typedOther = other;
    return _style == typedOther._style &&
           _sigma == typedOther._sigma;
  }

  @override
  int get hashCode => hashValues(_style, _sigma);

  @override
  String toString() => 'MaskFilter.blur($_style, ${_sigma.toStringAsFixed(1)})';
}

/// Stub implementation. See docs in `../ui/`.
class ColorFilter {
  /// Stub implementation. See docs in `../ui/`.
  const ColorFilter.mode(Color color, BlendMode blendMode)
      : _color = color,
        _blendMode = blendMode,
        _matrix = null,
        _type = _TypeMode;

  /// Stub implementation. See docs in `../ui/`.
  const ColorFilter.matrix(List<double> matrix)
      : _color = null,
        _blendMode = null,
        _matrix = matrix,
        _type = _TypeMatrix;

  /// Stub implementation. See docs in `../ui/`.
  const ColorFilter.linearToSrgbGamma()
      : _color = null,
        _blendMode = null,
        _matrix = null,
        _type = _TypeLinearToSrgbGamma;

  /// Stub implementation. See docs in `../ui/`.
  const ColorFilter.srgbToLinearGamma()
      : _color = null,
        _blendMode = null,
        _matrix = null,
        _type = _TypeSrgbToLinearGamma;

  final Color _color;
  final BlendMode _blendMode;
  final List<double> _matrix;
  final int _type;

  // The type of SkColorFilter class to create for Skia.
  // These constants must be kept in sync with ColorFilterType in paint.cc.
  static const int _TypeNone = 0; // null
  static const int _TypeMode = 1; // MakeModeFilter
  static const int _TypeMatrix = 2; // MakeMatrixFilterRowMajor255
  static const int _TypeLinearToSrgbGamma = 3; // MakeLinearToSRGBGamma
  static const int _TypeSrgbToLinearGamma = 4; // MakeSRGBToLinearGamma

  @override
  bool operator ==(dynamic other) {
    if (other is! ColorFilter) {
      return false;
    }
    final ColorFilter typedOther = other;

    if (_type != typedOther._type) {
      return false;
    }
    if (!_listEquals<double>(_matrix, typedOther._matrix)) {
      return false;
    }

    return _color == typedOther._color && _blendMode == typedOther._blendMode;
  }

  @override
  int get hashCode => hashValues(_color, _blendMode, hashList(_matrix), _type);

  @override
  String toString() {
    switch (_type) {
      case _TypeMode:
        return 'ColorFilter.mode($_color, $_blendMode)';
      case _TypeMatrix:
        return 'ColorFilter.matrix($_matrix)';
      case _TypeLinearToSrgbGamma:
        return 'ColorFilter.linearToSrgbGamma()';
      case _TypeSrgbToLinearGamma:
        return 'ColorFilter.srgbToLinearGamma()';
      default:
        return 'Unknown ColorFilter type. This is an error. If you\'re seeing this, please file an issue at https://github.com/flutter/flutter/issues/new.';
    }
  }
}

/// Stub implementation. See docs in `../ui/`.
class ImageFilter {

  /// Stub implementation. See docs in `../ui/`.
  ImageFilter.blur({ double sigmaX: 0.0, double sigmaY: 0.0 });

  /// Stub implementation. See docs in `../ui/`.
  ImageFilter.matrix(Float64List matrix4,
                     { FilterQuality filterQuality: FilterQuality.low });
}

/// Stub implementation. See docs in `../ui/`.
class Shader {
  /// Stub implementation. See docs in `../ui/`.
  Shader._();
}

/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkShader::TileMode.
enum TileMode {
  /// Stub implementation. See docs in `../ui/`.
  clamp,

  /// Stub implementation. See docs in `../ui/`.
  repeated,

  /// Stub implementation. See docs in `../ui/`.
  mirror,
}

/// Stub implementation. See docs in `../ui/`.
class Gradient extends Shader {

  /// Stub implementation. See docs in `../ui/`.
  Gradient.linear(
    Offset from,
    Offset to,
    List<Color> colors, [
    List<double> colorStops,
    TileMode tileMode = TileMode.clamp,
  ]) : assert(_offsetIsValid(from)),
       assert(_offsetIsValid(to)),
       assert(colors != null),
       assert(tileMode != null),
       super._();

  /// Stub implementation. See docs in `../ui/`.
  Gradient.radial(
    Offset center,
    double radius,
    List<Color> colors, [
    List<double> colorStops,
    TileMode tileMode = TileMode.clamp,
    Float64List matrix4,
    Offset focal,
    double focalRadius = 0.0
  ]) : assert(_offsetIsValid(center)),
       assert(colors != null),
       assert(tileMode != null),
       assert(matrix4 == null || _matrix4IsValid(matrix4)),
       super._();

  /// Stub implementation. See docs in `../ui/`.
  Gradient.sweep(
    Offset center,
    List<Color> colors, [
    List<double> colorStops,
    TileMode tileMode = TileMode.clamp,
    double startAngle = 0.0,
    double endAngle = math.pi * 2,
    Float64List matrix4,
  ]) : assert(_offsetIsValid(center)),
       assert(colors != null),
       assert(tileMode != null),
       assert(startAngle != null),
       assert(endAngle != null),
       assert(startAngle < endAngle),
       assert(matrix4 == null || _matrix4IsValid(matrix4)),
       super._();
}

/// Stub implementation. See docs in `../ui/`.
class ImageShader extends Shader {
  /// Stub implementation. See docs in `../ui/`.
  ImageShader(Image image, TileMode tmx, TileMode tmy, Float64List matrix4) :
    assert(image != null), // image is checked on the engine side
    assert(tmx != null),
    assert(tmy != null),
    assert(matrix4 != null),
    super._();
}

/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkVertices::VertexMode.
enum VertexMode {
  /// Stub implementation. See docs in `../ui/`.
  triangles,

  /// Stub implementation. See docs in `../ui/`.
  triangleStrip,

  /// Stub implementation. See docs in `../ui/`.
  triangleFan,
}

/// Stub implementation. See docs in `../ui/`.
class Vertices {
  Vertices(
    VertexMode mode,
    List<Offset> positions, {
    List<Offset> textureCoordinates,
    List<Color> colors,
    List<int> indices,
  }) : assert(mode != null),
       assert(positions != null);

  Vertices.raw(
    VertexMode mode,
    Float32List positions, {
    Float32List textureCoordinates,
    Int32List colors,
    Int32List indices,
  }) : assert(mode != null),
       assert(positions != null);
}

/// Stub implementation. See docs in `../ui/`.
// ignore: deprecated_member_use
/// Stub implementation. See docs in `../ui/`.
// These enum values must be kept in sync with SkCanvas::PointMode.
enum PointMode {
  /// Stub implementation. See docs in `../ui/`.
  points,

  /// Stub implementation. See docs in `../ui/`.
  lines,

  /// Stub implementation. See docs in `../ui/`.
  polygon,
}

/// Stub implementation. See docs in `../ui/`.
enum ClipOp {
  /// Stub implementation. See docs in `../ui/`.
  difference,

  /// Stub implementation. See docs in `../ui/`.
  intersect,
}

/// Stub implementation. See docs in `../ui/`.
class Canvas {
  /// Stub implementation. See docs in `../ui/`.
  Canvas(PictureRecorder recorder, [ Rect cullRect ]) : assert(recorder != null);

  /// Stub implementation. See docs in `../ui/`.
  void save() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void saveLayer(Rect bounds, Paint paint) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void restore() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  int getSaveCount() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void translate(double dx, double dy) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void scale(double sx, [double sy]) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void rotate(double radians) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void skew(double sx, double sy) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void transform(Float64List matrix4) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void clipRect(Rect rect, { ClipOp clipOp: ClipOp.intersect, bool doAntiAlias = true }) {
    assert(_rectIsValid(rect));
    assert(clipOp != null);
    assert(doAntiAlias != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void clipRRect(RRect rrect, {bool doAntiAlias = true}) {
    assert(_rrectIsValid(rrect));
    assert(doAntiAlias != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void clipPath(Path path, {bool doAntiAlias = true}) {
    assert(path != null); // path is checked on the engine side
    assert(doAntiAlias != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawColor(Color color, BlendMode blendMode) {
    assert(color != null);
    assert(blendMode != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawLine(Offset p1, Offset p2, Paint paint) {
    assert(_offsetIsValid(p1));
    assert(_offsetIsValid(p2));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawPaint(Paint paint) {
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawRect(Rect rect, Paint paint) {
    assert(_rectIsValid(rect));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawRRect(RRect rrect, Paint paint) {
    assert(_rrectIsValid(rrect));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawDRRect(RRect outer, RRect inner, Paint paint) {
    assert(_rrectIsValid(outer));
    assert(_rrectIsValid(inner));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawOval(Rect rect, Paint paint) {
    assert(_rectIsValid(rect));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawCircle(Offset c, double radius, Paint paint) {
    assert(_offsetIsValid(c));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawArc(Rect rect, double startAngle, double sweepAngle, bool useCenter, Paint paint) {
    assert(_rectIsValid(rect));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawPath(Path path, Paint paint) {
    assert(path != null); // path is checked on the engine side
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawImage(Image image, Offset p, Paint paint) {
    assert(image != null); // image is checked on the engine side
    assert(_offsetIsValid(p));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawImageRect(Image image, Rect src, Rect dst, Paint paint) {
    assert(image != null); // image is checked on the engine side
    assert(_rectIsValid(src));
    assert(_rectIsValid(dst));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawImageNine(Image image, Rect center, Rect dst, Paint paint) {
    assert(image != null); // image is checked on the engine side
    assert(_rectIsValid(center));
    assert(_rectIsValid(dst));
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawPicture(Picture picture) {
    assert(picture != null); // picture is checked on the engine side
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawParagraph(Paragraph paragraph, Offset offset) {
    assert(paragraph != null);
    assert(_offsetIsValid(offset));
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawPoints(PointMode pointMode, List<Offset> points, Paint paint) {
    assert(pointMode != null);
    assert(points != null);
    assert(paint != null);
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawRawPoints(PointMode pointMode, Float32List points, Paint paint) {
    assert(pointMode != null);
    assert(points != null);
    assert(paint != null);
    throw UnimplementedError();
  }

  void drawVertices(Vertices vertices, BlendMode blendMode, Paint paint) {
    assert(vertices != null); // vertices is checked on the engine side
    assert(paint != null);
    assert(blendMode != null);
    throw UnimplementedError();
  }

  //
  // See also:
  //
  //  * [drawRawAtlas], which takes its arguments as typed data lists rather
  //    than objects.
  void drawAtlas(Image atlas,
                 List<RSTransform> transforms,
                 List<Rect> rects,
                 List<Color> colors,
                 BlendMode blendMode,
                 Rect cullRect,
                 Paint paint) {
    assert(atlas != null); // atlas is checked on the engine side
    assert(transforms != null);
    assert(rects != null);
    assert(colors != null);
    assert(blendMode != null);
    assert(paint != null);
    throw UnimplementedError();
  }

  //
  // The `rstTransforms` argument is interpreted as a list of four-tuples, with
  // each tuple being ([RSTransform.scos], [RSTransform.ssin],
  // [RSTransform.tx], [RSTransform.ty]).
  //
  // The `rects` argument is interpreted as a list of four-tuples, with each
  // tuple being ([Rect.left], [Rect.top], [Rect.right], [Rect.bottom]).
  //
  // The `colors` argument, which can be null, is interpreted as a list of
  // 32-bit colors, with the same packing as [Color.value].
  //
  // See also:
  //
  //  * [drawAtlas], which takes its arguments as objects rather than typed
  //    data lists.
  void drawRawAtlas(Image atlas,
                    Float32List rstTransforms,
                    Float32List rects,
                    Int32List colors,
                    BlendMode blendMode,
                    Rect cullRect,
                    Paint paint) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void drawShadow(Path path, Color color, double elevation, bool transparentOccluder) {
    assert(path != null); // path is checked on the engine side
    assert(color != null);
    assert(transparentOccluder != null);
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class Picture {
  /// Stub implementation. See docs in `../ui/`.
  Picture._();

  /// Stub implementation. See docs in `../ui/`.
  Future<Image> toImage(int width, int height) {
    throw UnimplementedError();
  }
  /// Stub implementation. See docs in `../ui/`.
  void dispose() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  int get approximateBytesUsed {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class PictureRecorder {
  /// Stub implementation. See docs in `../ui/`.
  PictureRecorder();

  /// Stub implementation. See docs in `../ui/`.
  bool get isRecording {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Picture endRecording() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class Shadow {
  /// Stub implementation. See docs in `../ui/`.
  const Shadow({
    this.color = const Color(_kColorDefault),
    this.offset = Offset.zero,
    this.blurRadius = 0.0,
  }) : assert(color != null, 'Text shadow color was null.'),
       assert(offset != null, 'Text shadow offset was null.'),
       assert(blurRadius >= 0.0, 'Text shadow blur radius should be non-negative.');

  static const int _kColorDefault = 0xFF000000;

  /// Stub implementation. See docs in `../ui/`.
  final Color color;

  /// Stub implementation. See docs in `../ui/`.
  final Offset offset;

  /// Stub implementation. See docs in `../ui/`.
  final double blurRadius;

  /// Stub implementation. See docs in `../ui/`.
  // See SkBlurMask::ConvertRadiusToSigma().
  // <https://github.com/google/skia/blob/bb5b77db51d2e149ee66db284903572a5aac09be/src/effects/SkBlurMask.cpp#L23>
  static double convertRadiusToSigma(double radius) {
    return radius * 0.57735 + 0.5;
  }

  /// Stub implementation. See docs in `../ui/`.
  double get blurSigma => convertRadiusToSigma(blurRadius);

  /// Stub implementation. See docs in `../ui/`.
  Paint toPaint() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Shadow scale(double factor) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  static Shadow lerp(Shadow a, Shadow b, double t) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  static List<Shadow> lerpList(List<Shadow> a, List<Shadow> b, double t) {
    throw UnimplementedError();
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other is! Shadow)
      return false;
    final Shadow typedOther = other;
    return color == typedOther.color &&
           offset == typedOther.offset &&
           blurRadius == typedOther.blurRadius;
  }

  @override
  int get hashCode => hashValues(color, offset, blurRadius);

  @override
  String toString() => 'TextShadow($color, $offset, $blurRadius)';
}

/// Stub implementation. See docs in `../ui/`.
typedef _Callback<T> = void Function(T result);

/// Stub implementation. See docs in `../ui/`.
typedef _Callbacker<T> = String Function(_Callback<T> callback);

/// Stub implementation. See docs in `../ui/`.
Future<T> _futurize<T>(_Callbacker<T> callbacker) {
  throw UnimplementedError();
}
