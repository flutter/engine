import 'dart:async';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/canvaskit/renderer.dart';
import 'package:ui/src/engine/fonts.dart';
import 'package:ui/src/engine/html/renderer.dart';
import 'package:ui/ui.dart';

final Renderer _renderer = Renderer._internal();
Renderer get renderer => _renderer;

abstract class Renderer {
  factory Renderer._internal() {
    bool useCanvasKit;
    if (FlutterConfiguration.flutterWebAutoDetect) {
      if (requestedRendererType != null) {
        useCanvasKit = requestedRendererType! == 'canvaskit';
      } else {
        // If requestedRendererType is not specified, use CanvasKit for desktop and
        // html for mobile.
        useCanvasKit = isDesktop;
      }
    } else {
      useCanvasKit = FlutterConfiguration.useSkia;
    }

    return useCanvasKit ? CanvasKitRenderer() : HtmlRenderer();
  }

  String get rendererTag;
  FontCollection get fontCollection;

  FutureOr<void> initialize();
  void reset();

  Paint createPaint();

  Vertices createVertices(
    VertexMode mode,
    List<Offset> positions, {
    List<Offset>? textureCoordinates,
    List<Color>? colors,
    List<int>? indices,
  });
  Vertices createVerticesRaw(
    VertexMode mode,
    Float32List positions, {
    Float32List? textureCoordinates,
    Int32List? colors,
    Uint16List? indices,
  });

  PictureRecorder createPictureRecorder();
  Canvas createCanvas(PictureRecorder recorder, [Rect? cullRect]);
  SceneBuilder createSceneBuilder();

  Gradient createLinearGradient(
    Offset from,
    Offset to,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix4,
  ]);
  Gradient createRadialGradient(
    Offset center,
    double radius,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix4,
  ]);
  Gradient createConicalGradient(
    Offset focal,
    double focalRadius,
    Offset center,
    double radius,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix,
  ]);
  Gradient createSweepGradient(
    Offset center,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    double startAngle = 0.0,
    double endAngle = math.pi * 2,
    Float32List? matrix4,
  ]);

  ImageFilter createBlurImageFilter({
    double sigmaX = 0.0, 
    double sigmaY = 0.0, 
    TileMode tileMode = TileMode.clamp});
  ImageFilter createDilateImageFilter({ double radiusX = 0.0, double radiusY = 0.0});
  ImageFilter createErodeImageFilter({ double radiusX = 0.0, double radiusY = 0.0});
  ImageFilter createMatrixImageFilter( 
    Float64List matrix4, {
    FilterQuality filterQuality = FilterQuality.low
  });
  ImageFilter composeImageFilters({required ImageFilter outer, required ImageFilter inner});

  Future<Codec> instantiateImageCodec(
    Uint8List list, {
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true,
  });
  Future<Codec> instantiateImageCodecFromUrl(
    Uri uri, {
    WebOnlyImageCodecChunkCallback? chunkCallback,
  });

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
  });

  ImageShader createImageShader(
    Image image,
    TileMode tmx,
    TileMode tmy,
    Float64List matrix4, 
    FilterQuality? filterQuality,
  );

  Path createPath();
  Path copyPath(Path src);
  Path combinePaths(PathOperation op, Path path1, Path path2);

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
    List<FontVariation>? fontVariations,
  });

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
    Locale? locale,
  });

  StrutStyle createStrutStyle({
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? height,
    TextLeadingDistribution? leadingDistribution,
    double? leading,
    FontWeight? fontWeight,
    FontStyle? fontStyle,
    bool? forceStrutHeight,
  });

  ParagraphBuilder createParagraphBuilder(ParagraphStyle style);

  void renderScene(Scene scene);
}
