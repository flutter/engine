import 'dart:async';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';

import '../fonts.dart';

class HtmlRenderer implements Renderer {
  @override
  String get rendererTag => 'html';

  late final FontCollection _fontCollection = HtmlFontCollection();
  
  @override
  FontCollection get fontCollection => _fontCollection;

  @override
  void initialize() {
    scheduleMicrotask(() {
      // Access [lineLookup] to force the lazy unpacking of line break data
      // now. Removing this line won't break anything. It's just an optimization
      // to make the unpacking happen while we are waiting for network requests.
      lineLookup;
    });
  }

  @override
  void reset() {}

  @override
  Paint createPaint() => SurfacePaint();

  @override
  Vertices createVertices(
    VertexMode mode,
    List<Offset> positions, {
    List<Offset>? textureCoordinates,
    List<Color>? colors,
    List<int>? indices,
  }) => SurfaceVertices(
    mode,
    positions,
    colors: colors,
    indices: indices);

  @override
  Vertices createVerticesRaw(
    VertexMode mode,
    Float32List positions, {
    Float32List? textureCoordinates,
    Int32List? colors,
    Uint16List? indices,
  }) => SurfaceVertices.raw(
    mode,
    positions,
    colors: colors,
    indices: indices);

  @override
  Canvas createCanvas(PictureRecorder recorder, [Rect? cullRect]) =>
    SurfaceCanvas(recorder as EnginePictureRecorder, cullRect);

  @override
  Gradient createLinearGradient(
    Offset from,
    Offset to,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix4
  ]) => GradientLinear(from, to, colors, colorStops, tileMode, matrix4);

  @override
  Gradient createRadialGradient(
    Offset center,
    double radius,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix4,
    Offset? focal,
    double focalRadius = 0.0,
  ]) => GradientRadial(center, radius, colors, colorStops, tileMode, matrix4);

  @override
  Gradient createConicalGradient(
    Offset focal,
    double focalRadius,
    Offset center,
    double radius,
    List<Color> colors,
    [List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix
  ]) => GradientConical(
    focal,
    focalRadius,
    center,
    radius,
    colors,
    colorStops,
    tileMode,
    matrix);

  @override
  Gradient createSweepGradient(
    Offset center,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    double startAngle = 0.0,
    double endAngle = math.pi * 2,
    Float32List? matrix4
  ]) => GradientSweep(center, colors, colorStops, tileMode, startAngle, endAngle, matrix4);

  @override
  PictureRecorder createPictureRecorder() => EnginePictureRecorder();

  @override
  SceneBuilder createSceneBuilder() => SurfaceSceneBuilder();
    
  // TODO(ferhat): implement TileMode.
  @override
  ImageFilter createBlurImageFilter({
    double sigmaX = 0.0, 
    double sigmaY = 0.0, 
    TileMode tileMode = TileMode.clamp
  }) => EngineImageFilter.blur(sigmaX: sigmaX, sigmaY: sigmaY, tileMode: tileMode);
  
  @override
  ImageFilter createDilateImageFilter({double radiusX = 0.0, double radiusY = 0.0}) {
    // TODO(fzyzcjy): implement dilate. https://github.com/flutter/flutter/issues/101085
    throw UnimplementedError('ImageFilter.dilate not implemented for HTML renderer.');
  }
  
  @override
  ImageFilter createErodeImageFilter({double radiusX = 0.0, double radiusY = 0.0}) {
    // TODO(fzyzcjy): implement erode. https://github.com/flutter/flutter/issues/101085
    throw UnimplementedError('ImageFilter.erode not implemented for HTML renderer.');
  }
  
  @override
  ImageFilter createMatrixImageFilter(
    Float64List matrix4, {
    FilterQuality filterQuality = FilterQuality.low
  }) => EngineImageFilter.matrix(matrix: matrix4, filterQuality: filterQuality);

  @override
  ImageFilter composeImageFilters({required ImageFilter outer, required ImageFilter inner}) {
    // TODO(ferhat): add implementation and remove the "ignore".
    // ignore: avoid_unused_constructor_parameters
    throw UnimplementedError('ImageFilter.erode not implemented for HTML renderer.');
  }
  
  @override
  Future<Codec> instantiateImageCodec(
    Uint8List list, {
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true}) async {
    final DomBlob blob = createDomBlob(<dynamic>[list.buffer]);
    return HtmlBlobCodec(blob);
  }
  
  @override
  Future<Codec> instantiateImageCodecFromUrl(
    Uri uri, {
    WebOnlyImageCodecChunkCallback? chunkCallback}) {
      return futurize<Codec>((Callback<Codec> callback) {
        callback(HtmlCodec(uri.toString(), chunkCallback: chunkCallback));
      });
  }
  
  @override
  void decodeImageFromPixels(
    Uint8List pixels,
    int width,
    int height,
    PixelFormat format,
    ImageDecoderCallback callback, {
    int? rowBytes,
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true
  }) {
    void executeCallback(Codec codec) {
      codec.getNextFrame().then((FrameInfo frameInfo) {
        callback(frameInfo.image);
      });
    }
    createBmp(pixels, width, height, rowBytes ?? width, format).then(
      executeCallback);
  }

  @override
  ImageShader createImageShader(
    Image image,
    TileMode tmx,
    TileMode tmy,
    Float64List matrix4,
    FilterQuality? filterQuality
  ) => EngineImageShader(image, tmx, tmy, matrix4, filterQuality);

  @override
  Path createPath() => SurfacePath();

  @override
  Path copyPath(Path src) => SurfacePath.from(src as SurfacePath);

  @override
  Path combinePaths(PathOperation op, Path path1, Path path2) {
    throw UnimplementedError('combinePaths not implemented in HTML renderer.');
  }

  @override
  TextStyle createTextStyle({
    Color? color,
    TextDecoration? decoration,
    Color? decorationColor,
    TextDecorationStyle? decorationStyle, 
    double? decorationThickness,
    FontWeight? fontWeight,
    FontStyle? fontStyle,
    TextBaseline? textBaseline,
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? letterSpacing,
    double? wordSpacing,
    double? height,
    TextLeadingDistribution? leadingDistribution,
    Locale? locale,
    Paint? background,
    Paint? foreground,
    List<Shadow>? shadows,
    List<FontFeature>? fontFeatures,
    List<FontVariation>? fontVariations
  }) => EngineTextStyle(
    color: color,
    decoration: decoration,
    decorationColor: decorationColor,
    decorationStyle: decorationStyle,
    decorationThickness: decorationThickness,
    fontWeight: fontWeight,
    fontStyle: fontStyle,
    textBaseline: textBaseline,
    fontFamily: fontFamily,
    fontFamilyFallback: fontFamilyFallback,
    fontSize: fontSize,
    letterSpacing: letterSpacing,
    wordSpacing: wordSpacing,
    height: height,
    locale: locale,
    background: background,
    foreground: foreground,
    shadows: shadows,
    fontFeatures: fontFeatures,
    fontVariations: fontVariations,
  );

  @override
  ParagraphStyle createParagraphStyle({
    TextAlign? textAlign,
    TextDirection? textDirection,
    int? maxLines,
    String? fontFamily,
    double? fontSize,
    double? height,
    TextHeightBehavior? textHeightBehavior,
    FontWeight? fontWeight,
    FontStyle? fontStyle,
    StrutStyle? strutStyle,
    String? ellipsis,
    Locale? locale
  }) => EngineParagraphStyle(
    textAlign: textAlign,
    textDirection: textDirection,
    maxLines: maxLines,
    fontFamily: fontFamily,
    fontSize: fontSize,
    height: height,
    textHeightBehavior: textHeightBehavior,
    fontWeight: fontWeight,
    fontStyle: fontStyle,
    strutStyle: strutStyle,
    ellipsis: ellipsis,
    locale: locale,
  );
  
  @override
  StrutStyle createStrutStyle({
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? height,
    TextLeadingDistribution? leadingDistribution,
    double? leading,
    FontWeight? fontWeight,
    FontStyle? fontStyle,
    bool? forceStrutHeight
  }) => EngineStrutStyle(
    fontFamily: fontFamily,
    fontFamilyFallback: fontFamilyFallback,
    fontSize: fontSize,
    height: height,
    leadingDistribution: leadingDistribution,
    leading: leading,
    fontWeight: fontWeight,
    fontStyle: fontStyle,
    forceStrutHeight: forceStrutHeight,
  );

  @override
  ParagraphBuilder createParagraphBuilder(ParagraphStyle style) => 
    CanvasParagraphBuilder(style as EngineParagraphStyle);
    
  @override
  void renderScene(Scene scene) {
    flutterViewEmbedder.addSceneToSceneHost((scene as SurfaceScene).webOnlyRootElement);
    frameTimingsOnRasterFinish();
  }
}
