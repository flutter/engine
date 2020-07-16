// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9
part of ui;

// ignore: unused_element, Used in Shader assert.
bool _offsetIsValid(Offset offset) {
  assert(offset != null, 'Offset argument was null.'); // ignore: unnecessary_null_comparison
  assert(!offset.dx.isNaN && !offset.dy.isNaN,
      'Offset argument contained a NaN value.');
  return true;
}

// ignore: unused_element, Used in Shader assert.
bool _matrix4IsValid(Float32List matrix4) {
  assert(matrix4 != null, 'Matrix4 argument was null.'); // ignore: unnecessary_null_comparison
  assert(matrix4.length == 16, 'Matrix4 must have 16 entries.');
  return true;
}

void _validateColorStops(List<Color> colors, List<double>? colorStops) {
  if (colorStops == null) {
    if (colors.length != 2)
      throw ArgumentError(
          '"colors" must have length 2 if "colorStops" is omitted.');
  } else {
    if (colors.length != colorStops.length)
      throw ArgumentError(
          '"colors" and "colorStops" arguments must have equal length.');
  }
}

Color _scaleAlpha(Color a, double factor) {
  return a.withAlpha(_clampInt((a.alpha * factor).round(), 0, 255));
}

class Color {
  const Color(int value) : this.value = value & 0xFFFFFFFF;

  const Color.fromARGB(int a, int r, int g, int b)
      : value = (((a & 0xff) << 24) |
                ((r & 0xff) << 16) |
                ((g & 0xff) << 8) |
                ((b & 0xff) << 0)) &
            0xFFFFFFFF;

  const Color.fromRGBO(int r, int g, int b, double opacity)
      : value = ((((opacity * 0xff ~/ 1) & 0xff) << 24) |
                ((r & 0xff) << 16) |
                ((g & 0xff) << 8) |
                ((b & 0xff) << 0)) &
            0xFFFFFFFF;

  final int value;

  int get alpha => (0xff000000 & value) >> 24;

  double get opacity => alpha / 0xFF;

  int get red => (0x00ff0000 & value) >> 16;

  int get green => (0x0000ff00 & value) >> 8;

  int get blue => (0x000000ff & value) >> 0;

  Color withAlpha(int a) {
    return Color.fromARGB(a, red, green, blue);
  }

  Color withOpacity(double opacity) {
    assert(opacity >= 0.0 && opacity <= 1.0);
    return withAlpha((255.0 * opacity).round());
  }

  Color withRed(int r) {
    return Color.fromARGB(alpha, r, green, blue);
  }

  Color withGreen(int g) {
    return Color.fromARGB(alpha, red, g, blue);
  }

  Color withBlue(int b) {
    return Color.fromARGB(alpha, red, green, b);
  }

  // See <https://www.w3.org/TR/WCAG20/#relativeluminancedef>
  static double _linearizeColorComponent(double component) {
    if (component <= 0.03928) {
      return component / 12.92;
    }
    return math.pow((component + 0.055) / 1.055, 2.4) as double;
  }

  double computeLuminance() {
    // See <https://www.w3.org/TR/WCAG20/#relativeluminancedef>
    final double R = _linearizeColorComponent(red / 0xFF);
    final double G = _linearizeColorComponent(green / 0xFF);
    final double B = _linearizeColorComponent(blue / 0xFF);
    return 0.2126 * R + 0.7152 * G + 0.0722 * B;
  }

  static Color? lerp(Color? a, Color? b, double t) {
    assert(t != null); // ignore: unnecessary_null_comparison
    if (b == null) {
      if (a == null) {
        return null;
      } else {
        return _scaleAlpha(a, 1.0 - t);
      }
    } else {
      if (a == null) {
        return _scaleAlpha(b, t);
      } else {
        return Color.fromARGB(
          _clampInt(_lerpInt(a.alpha, b.alpha, t).toInt(), 0, 255),
          _clampInt(_lerpInt(a.red, b.red, t).toInt(), 0, 255),
          _clampInt(_lerpInt(a.green, b.green, t).toInt(), 0, 255),
          _clampInt(_lerpInt(a.blue, b.blue, t).toInt(), 0, 255),
        );
      }
    }
  }

  static Color alphaBlend(Color foreground, Color background) {
    final int alpha = foreground.alpha;
    if (alpha == 0x00) {
      // Foreground completely transparent.
      return background;
    }
    final int invAlpha = 0xff - alpha;
    int backAlpha = background.alpha;
    if (backAlpha == 0xff) {
      // Opaque background case
      return Color.fromARGB(
        0xff,
        (alpha * foreground.red + invAlpha * background.red) ~/ 0xff,
        (alpha * foreground.green + invAlpha * background.green) ~/ 0xff,
        (alpha * foreground.blue + invAlpha * background.blue) ~/ 0xff,
      );
    } else {
      // General case
      backAlpha = (backAlpha * invAlpha) ~/ 0xff;
      final int outAlpha = alpha + backAlpha;
      assert(outAlpha != 0x00);
      return Color.fromARGB(
        outAlpha,
        (foreground.red * alpha + background.red * backAlpha) ~/ outAlpha,
        (foreground.green * alpha + background.green * backAlpha) ~/ outAlpha,
        (foreground.blue * alpha + background.blue * backAlpha) ~/ outAlpha,
      );
    }
  }

  static int getAlphaFromOpacity(double opacity) {
    assert(opacity != null); // ignore: unnecessary_null_comparison
    return (opacity.clamp(0.0, 1.0) * 255).round();
  }

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    if (other.runtimeType != runtimeType) {
      return false;
    }
    return other is Color
        && other.value == value;
  }

  @override
  int get hashCode => value.hashCode;

  @override
  String toString() {
    return 'Color(0x${value.toRadixString(16).padLeft(8, '0')})';
  }
}

enum StrokeCap {
  butt,
  round,
  square,
}

// These enum values must be kept in sync with SkPaint::Join.
enum StrokeJoin {
  miter,
  round,
  bevel,
}

enum PaintingStyle {
  fill,
  stroke,
}

enum BlendMode {
  clear,
  src,
  dst,
  srcOver,
  dstOver,
  srcIn,
  dstIn,
  srcOut,
  dstOut,
  srcATop,
  dstATop,
  xor,
  plus,
  modulate,
  screen,
  overlay,
  darken,
  lighten,
  colorDodge,
  colorBurn,
  hardLight,
  softLight,
  difference,
  exclusion,
  multiply,
  hue,
  saturation,
  color,
  luminosity,
}

enum Clip {
  none,
  hardEdge,
  antiAlias,
  antiAliasWithSaveLayer,
}

abstract class Paint {
  factory Paint() =>
      engine.experimentalUseSkia ? engine.CkPaint() : engine.SurfacePaint();

  static bool enableDithering = false;

  BlendMode get blendMode;
  set blendMode(BlendMode value);

  PaintingStyle get style;
  set style(PaintingStyle value);

  double get strokeWidth;
  set strokeWidth(double value);

  StrokeCap get strokeCap;
  set strokeCap(StrokeCap value);

  StrokeJoin get strokeJoin;
  set strokeJoin(StrokeJoin value);

  bool get isAntiAlias;
  set isAntiAlias(bool value);

  Color get color;
  set color(Color value);

  bool get invertColors;

  set invertColors(bool value);

  Shader? get shader;
  set shader(Shader? value);

  MaskFilter? get maskFilter;
  set maskFilter(MaskFilter? value);

  // TODO(ianh): verify that the image drawing methods actually respect this
  FilterQuality get filterQuality;
  set filterQuality(FilterQuality value);

  ColorFilter? get colorFilter;
  set colorFilter(ColorFilter? value);

  double get strokeMiterLimit;
  set strokeMiterLimit(double value);

  ImageFilter? get imageFilter;
  set imageFilter(ImageFilter? value);
}

abstract class Shader {
  Shader._();
}

abstract class Gradient extends Shader {
  factory Gradient.linear(
    Offset from,
    Offset to,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float64List? matrix4,
  ]) =>
      engine.GradientLinear(from, to, colors, colorStops, tileMode, matrix4);

  factory Gradient.radial(
    Offset center,
    double radius,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float64List? matrix4,
    Offset? focal,
    double focalRadius = 0.0,
  ]) {
    _validateColorStops(colors, colorStops);
    // If focal is null or focal radius is null, this should be treated as a regular radial gradient
    // If focal == center and the focal radius is 0.0, it's still a regular radial gradient
    final Float32List? matrix32 = matrix4 != null ? engine.toMatrix32(matrix4) : null;
    if (focal == null || (focal == center && focalRadius == 0.0)) {
      return engine.GradientRadial(
          center, radius, colors, colorStops, tileMode, matrix32);
    } else {
      assert(center != Offset.zero ||
          focal != Offset.zero); // will result in exception(s) in Skia side
      return engine.GradientConical(focal, focalRadius, center, radius, colors,
          colorStops, tileMode, matrix32);
    }
  }

  factory Gradient.sweep(
    Offset center,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    double startAngle/*?*/ = 0.0,
    double endAngle/*!*/ = math.pi * 2,
    Float64List? matrix4,
  ]) =>
      engine.GradientSweep(
          center, colors, colorStops, tileMode, startAngle, endAngle, matrix4 != null ? engine.toMatrix32(matrix4) : null);
}

abstract class Image {
  int get width;

  int get height;

  Future<ByteData?> toByteData(
      {ImageByteFormat format = ImageByteFormat.rawRgba});

  void dispose();

  @override
  String toString() => '[$width\u00D7$height]';
}

abstract class ColorFilter {
  const factory ColorFilter.mode(Color color, BlendMode blendMode) =
      engine.EngineColorFilter.mode;

  const factory ColorFilter.matrix(List<double> matrix) =
      engine.EngineColorFilter.matrix;

  const factory ColorFilter.linearToSrgbGamma() =
      engine.EngineColorFilter.linearToSrgbGamma;

  const factory ColorFilter.srgbToLinearGamma() =
      engine.EngineColorFilter.srgbToLinearGamma;

  List<dynamic> webOnlySerializeToCssPaint() {
    throw UnsupportedError('ColorFilter for CSS paint not yet supported');
  }
}

enum BlurStyle {
  normal,
  solid,
  outer,
  inner,
}

class MaskFilter {
  const MaskFilter.blur(
    this._style,
    this._sigma,
  )   : assert(_style != null), // ignore: unnecessary_null_comparison
        assert(_sigma != null); // ignore: unnecessary_null_comparison

  final BlurStyle _style;
  final double _sigma;

  double get webOnlySigma => _sigma;

  BlurStyle get webOnlyBlurStyle => _style;

  @override
  bool operator ==(Object other) {
    return other is MaskFilter
        && other._style == _style
        && other._sigma == _sigma;
  }

  @override
  int get hashCode => hashValues(_style, _sigma);

  List<dynamic> webOnlySerializeToCssPaint() {
    return <dynamic>[_style.index, _sigma];
  }

  @override
  String toString() => 'MaskFilter.blur($_style, ${_sigma.toStringAsFixed(1)})';
}

enum FilterQuality {
  none,
  low,
  medium,
  high,
}

class ImageFilter {
  factory ImageFilter.blur({double sigmaX = 0.0, double sigmaY = 0.0}) {
    if (engine.experimentalUseSkia) {
      return engine.CkImageFilter.blur(sigmaX: sigmaX, sigmaY: sigmaY);
    }
    return engine.EngineImageFilter.blur(sigmaX: sigmaX, sigmaY: sigmaY);
  }

  ImageFilter.matrix(Float64List matrix4,
      {FilterQuality filterQuality = FilterQuality.low}) {
    // TODO(flutter_web): add implementation.
    throw UnimplementedError(
        'ImageFilter.matrix not implemented for web platform.');
    //    if (matrix4.length != 16)
    //      throw ArgumentError('"matrix4" must have 16 entries.');
  }
}

enum ImageByteFormat {
  rawRgba,
  rawUnmodified,
  png,
}

enum PixelFormat {
  rgba8888,
  bgra8888,
}

class _ImageInfo {
  _ImageInfo(this.width, this.height, this.format, int? rowBytes) : this.rowBytes = rowBytes ?? width * 4;

  int width;
  int height;
  int format;
  int rowBytes;
}

typedef ImageDecoderCallback = void Function(Image result);

abstract class FrameInfo {
  FrameInfo._();

  Duration get duration => Duration(milliseconds: _durationMillis);
  int get _durationMillis => 0;

  Image get image;
}

class Codec {
  Codec._();

  int get frameCount => 0;

  int get repetitionCount => 0;

  Future<FrameInfo> getNextFrame() {
    return engine.futurize<FrameInfo>(_getNextFrame);
  }

  String? _getNextFrame(engine.Callback<FrameInfo> callback) => null;

  void dispose() {}
}

Future<Codec> instantiateImageCodec(
  Uint8List list, {
  int? targetWidth,
  int? targetHeight,
  bool allowUpscaling = true,
}) {
  return _futurize<Codec>((engine.Callback<Codec> callback) =>
      // TODO: Implement targetWidth and targetHeight support.
      _instantiateImageCodec(list, callback, null));
}

String? _instantiateImageCodec(
    Uint8List list, engine.Callback<Codec> callback, _ImageInfo? imageInfo) {
  if (engine.experimentalUseSkia) {
    if (imageInfo == null) {
      engine.skiaInstantiateImageCodec(list, callback);
    } else {
      engine.skiaInstantiateImageCodec(list, callback, imageInfo.width,
          imageInfo.height, imageInfo.format, imageInfo.rowBytes);
    }
    return null;
  }
  final html.Blob blob = html.Blob(<dynamic>[list.buffer]);
  callback(engine.HtmlBlobCodec(blob));
  return null;
}

Future<Codec?> webOnlyInstantiateImageCodecFromUrl(Uri uri,
    {engine.WebOnlyImageCodecChunkCallback? chunkCallback}) {
  return _futurize<Codec?>((engine.Callback<Codec> callback) =>
      _instantiateImageCodecFromUrl(uri, chunkCallback, callback));
}

String? _instantiateImageCodecFromUrl(
    Uri uri,
    engine.WebOnlyImageCodecChunkCallback? chunkCallback,
    engine.Callback<Codec> callback) {
  callback(engine.HtmlCodec(uri.toString(), chunkCallback: chunkCallback));
  return null;
}

void decodeImageFromList(Uint8List list, ImageDecoderCallback callback) {
  _decodeImageFromListAsync(list, callback);
}

Future<void> _decodeImageFromListAsync(
    Uint8List list, ImageDecoderCallback callback) async {
  final Codec codec = await instantiateImageCodec(list);
  final FrameInfo frameInfo = await codec.getNextFrame();
  callback(frameInfo.image);
}

void decodeImageFromPixels(
  Uint8List pixels,
  int width,
  int height,
  PixelFormat format,
  ImageDecoderCallback callback, {
  int? rowBytes,
  int? targetWidth,
  int? targetHeight,
  bool allowUpscaling = true,
}) {
  final _ImageInfo imageInfo =
      _ImageInfo(width, height, format.index, rowBytes);
  final Future<Codec> codecFuture = _futurize(
      (engine.Callback<Codec> callback) =>
          _instantiateImageCodec(pixels, callback, imageInfo));
  codecFuture
      .then((Codec codec) => codec.getNextFrame())
      .then((FrameInfo frameInfo) => callback(frameInfo.image));
}

class Shadow {
  const Shadow({
    this.color = const Color(_kColorDefault),
    this.offset = Offset.zero,
    this.blurRadius = 0.0,
  })  : assert(color != null, 'Text shadow color was null.'), // ignore: unnecessary_null_comparison
        assert(offset != null, 'Text shadow offset was null.'), // ignore: unnecessary_null_comparison
        assert(blurRadius >= 0.0,
            'Text shadow blur radius should be non-negative.');

  static const int _kColorDefault = 0xFF000000;

  final Color color;

  final Offset offset;

  final double blurRadius;

  // See SkBlurMask::ConvertRadiusToSigma().
  // <https://github.com/google/skia/blob/bb5b77db51d2e149ee66db284903572a5aac09be/src/effects/SkBlurMask.cpp#L23>
  static double convertRadiusToSigma(double radius) {
    return radius * 0.57735 + 0.5;
  }

  double get blurSigma => convertRadiusToSigma(blurRadius);

  Paint toPaint() {
    return Paint()
      ..color = color
      ..maskFilter = MaskFilter.blur(BlurStyle.normal, blurSigma);
  }

  Shadow scale(double factor) {
    return Shadow(
      color: color,
      offset: offset * factor,
      blurRadius: blurRadius * factor,
    );
  }

  static Shadow? lerp(Shadow? a, Shadow? b, double t) {
    assert(t != null); // ignore: unnecessary_null_comparison
    if (b == null) {
      if (a == null) {
        return null;
      } else {
        return a.scale(1.0 - t);
      }
    } else {
      if (a == null) {
        return b.scale(t);
      } else {
        return Shadow(
          color: Color.lerp(a.color, b.color, t)!,
          offset: Offset.lerp(a.offset, b.offset, t)!,
          blurRadius: _lerpDouble(a.blurRadius, b.blurRadius, t),
        );
      }
    }
  }

  static List<Shadow>? lerpList(List<Shadow>? a, List<Shadow>? b, double t) {
    assert(t != null); // ignore: unnecessary_null_comparison
    if (a == null && b == null) {
      return null;
    }
    a ??= <Shadow>[];
    b ??= <Shadow>[];
    final List<Shadow> result = <Shadow>[];
    final int commonLength = math.min(a.length, b.length);
    for (int i = 0; i < commonLength; i += 1)
      result.add(Shadow.lerp(a[i], b[i], t)!);
    for (int i = commonLength; i < a.length; i += 1)
      result.add(a[i].scale(1.0 - t));
    for (int i = commonLength; i < b.length; i += 1) {
      result.add(b[i].scale(t));
    }
    return result;
  }

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    return other is Shadow
        && other.color == color
        && other.offset == offset
        && other.blurRadius == blurRadius;
  }

  @override
  int get hashCode => hashValues(color, offset, blurRadius);

  @override
  String toString() => 'TextShadow($color, $offset, $blurRadius)';
}


class ImageShader extends Shader {
  factory ImageShader(
    Image image,
    TileMode tmx,
    TileMode tmy,
    Float64List matrix4) {
    if (engine.experimentalUseSkia) {
      return engine.EngineImageShader(image, tmx, tmy, matrix4);
    }
    throw UnsupportedError(
        'ImageShader not implemented for web platform.');
  }
}
