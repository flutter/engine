import 'dart:async';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import '../../embedder.dart';
import '../../fonts.dart';
import '../../html_image_codec.dart';
import '../../renderer.dart';

class SkwasmRenderer implements Renderer {
  @override
  ui.Path combinePaths(ui.PathOperation op, ui.Path path1, ui.Path path2) {
    // TODO: implement combinePaths
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter composeImageFilters({required ui.ImageFilter outer, required ui.ImageFilter inner}) {
    // TODO: implement composeImageFilters
    throw UnimplementedError();
  }

  @override
  ui.Path copyPath(ui.Path src) {
    // TODO: implement copyPath
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter createBlurImageFilter({double sigmaX = 0.0, double sigmaY = 0.0, ui.TileMode tileMode = ui.TileMode.clamp}) {
    // TODO: implement createBlurImageFilter
    throw UnimplementedError();
  }

  @override
  ui.Canvas createCanvas(ui.PictureRecorder recorder, [ui.Rect? cullRect]) {
    // TODO: implement createCanvas
    throw UnimplementedError();
  }

  @override
  ui.Gradient createConicalGradient(ui.Offset focal, double focalRadius, ui.Offset center, double radius, List<ui.Color> colors, [List<double>? colorStops, ui.TileMode tileMode = ui.TileMode.clamp, Float32List? matrix]) {
    // TODO: implement createConicalGradient
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter createDilateImageFilter({double radiusX = 0.0, double radiusY = 0.0}) {
    // TODO: implement createDilateImageFilter
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter createErodeImageFilter({double radiusX = 0.0, double radiusY = 0.0}) {
    // TODO: implement createErodeImageFilter
    throw UnimplementedError();
  }

  @override
  ui.ImageShader createImageShader(ui.Image image, ui.TileMode tmx, ui.TileMode tmy, Float64List matrix4, ui.FilterQuality? filterQuality) {
    // TODO: implement createImageShader
    throw UnimplementedError();
  }

  @override
  ui.Gradient createLinearGradient(ui.Offset from, ui.Offset to, List<ui.Color> colors, [List<double>? colorStops, ui.TileMode tileMode = ui.TileMode.clamp, Float32List? matrix4]) {
    // TODO: implement createLinearGradient
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter createMatrixImageFilter(Float64List matrix4, {ui.FilterQuality filterQuality = ui.FilterQuality.low}) {
    // TODO: implement createMatrixImageFilter
    throw UnimplementedError();
  }

  @override
  ui.Paint createPaint() {
    // TODO: implement createPaint
    throw UnimplementedError();
  }

  @override
  ui.ParagraphBuilder createParagraphBuilder(ui.ParagraphStyle style) {
    // TODO: implement createParagraphBuilder
    throw UnimplementedError();
  }

  @override
  ui.ParagraphStyle createParagraphStyle({ui.TextAlign? textAlign, ui.TextDirection? textDirection, int? maxLines, String? fontFamily, double? fontSize, double? height, ui.TextHeightBehavior? textHeightBehavior, ui.FontWeight? fontWeight, ui.FontStyle? fontStyle, ui.StrutStyle? strutStyle, String? ellipsis, ui.Locale? locale}) {
    // TODO: implement createParagraphStyle
    throw UnimplementedError();
  }

  @override
  ui.Path createPath() {
    // TODO: implement createPath
    throw UnimplementedError();
  }

  @override
  ui.PictureRecorder createPictureRecorder() {
    // TODO: implement createPictureRecorder
    throw UnimplementedError();
  }

  @override
  ui.Gradient createRadialGradient(ui.Offset center, double radius, List<ui.Color> colors, [List<double>? colorStops, ui.TileMode tileMode = ui.TileMode.clamp, Float32List? matrix4]) {
    // TODO: implement createRadialGradient
    throw UnimplementedError();
  }

  @override
  ui.SceneBuilder createSceneBuilder() {
    // TODO: implement createSceneBuilder
    throw UnimplementedError();
  }

  @override
  ui.StrutStyle createStrutStyle({String? fontFamily, List<String>? fontFamilyFallback, double? fontSize, double? height, ui.TextLeadingDistribution? leadingDistribution, double? leading, ui.FontWeight? fontWeight, ui.FontStyle? fontStyle, bool? forceStrutHeight}) {
    // TODO: implement createStrutStyle
    throw UnimplementedError();
  }

  @override
  ui.Gradient createSweepGradient(ui.Offset center, List<ui.Color> colors, [List<double>? colorStops, ui.TileMode tileMode = ui.TileMode.clamp, double startAngle = 0.0, double endAngle = math.pi * 2, Float32List? matrix4]) {
    // TODO: implement createSweepGradient
    throw UnimplementedError();
  }

  @override
  ui.TextStyle createTextStyle({ui.Color? color, ui.TextDecoration? decoration, ui.Color? decorationColor, ui.TextDecorationStyle? decorationStyle, double? decorationThickness, ui.FontWeight? fontWeight, ui.FontStyle? fontStyle, ui.TextBaseline? textBaseline, String? fontFamily, List<String>? fontFamilyFallback, double? fontSize, double? letterSpacing, double? wordSpacing, double? height, ui.TextLeadingDistribution? leadingDistribution, ui.Locale? locale, ui.Paint? background, ui.Paint? foreground, List<ui.Shadow>? shadows, List<ui.FontFeature>? fontFeatures, List<ui.FontVariation>? fontVariations}) {
    // TODO: implement createTextStyle
    throw UnimplementedError();
  }

  @override
  ui.Vertices createVertices(ui.VertexMode mode, List<ui.Offset> positions, {List<ui.Offset>? textureCoordinates, List<ui.Color>? colors, List<int>? indices}) {
    // TODO: implement createVertices
    throw UnimplementedError();
  }

  @override
  ui.Vertices createVerticesRaw(ui.VertexMode mode, Float32List positions, {Float32List? textureCoordinates, Int32List? colors, Uint16List? indices}) {
    // TODO: implement createVerticesRaw
    throw UnimplementedError();
  }

  @override
  void decodeImageFromPixels(Uint8List pixels, int width, int height, ui.PixelFormat format, ui.ImageDecoderCallback callback, {int? rowBytes, int? targetWidth, int? targetHeight, bool allowUpscaling = true}) {
    // TODO: implement decodeImageFromPixels
  }

  @override
  // TODO: implement fontCollection
  FontCollection get fontCollection => throw UnimplementedError();

  @override
  FutureOr<void> initialize() {
    // TODO: implement initialize
    throw UnimplementedError();
  }

  @override
  Future<ui.Codec> instantiateImageCodec(Uint8List list, {int? targetWidth, int? targetHeight, bool allowUpscaling = true}) {
    // TODO: implement instantiateImageCodec
    throw UnimplementedError();
  }

  @override
  Future<ui.Codec> instantiateImageCodecFromUrl(Uri uri, {WebOnlyImageCodecChunkCallback? chunkCallback}) {
    // TODO: implement instantiateImageCodecFromUrl
    throw UnimplementedError();
  }

  @override
  void renderScene(ui.Scene scene) {
    // TODO: implement renderScene
  }

  @override
  // TODO: implement rendererTag
  String get rendererTag => throw UnimplementedError();

  @override
  void reset(FlutterViewEmbedder embedder) {
    // TODO: implement reset
  }
}
