// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine/html/shaders/shader.dart';
import 'package:ui/src/engine/picture.dart';
import 'package:ui/src/engine/text/canvas_paragraph.dart';
import 'package:ui/src/engine/text/paragraph.dart';
import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../embedder.dart';
import '../fonts.dart';
import '../html/canvas.dart';
import '../html/painting.dart';
import '../html/path/path.dart';
import '../html/render_vertices.dart';
import '../html/renderer.dart';
import '../html/shaders/image_shader.dart';
import '../html_image_codec.dart';
import '../profiler.dart';
import '../renderer.dart';
import '../util.dart';
import 'scene.dart';

class SceneletRenderer implements Renderer {
  static bool enabled = true;

  final HtmlRenderer _htmlRenderer = HtmlRenderer();
  late FlutterViewEmbedder _viewEmbedder;

  @override
  String get rendererTag => 'scenelet';

  @override
  FontCollection get fontCollection {
    return _htmlRenderer.fontCollection;
  }

  @override
  FutureOr<void> initialize() {
    _htmlRenderer.initialize();
  }

  @override
  ui.SceneBuilder createSceneBuilder() {
    return SceneletSceneBuilder();
  }

  @override
  void renderScene(ui.Scene scene) {
    // _viewEmbedder.addSceneToSceneHost((scene as SceneletScene).rootElement);
    frameTimingsOnRasterFinish();
  }

  @override
  void reset(FlutterViewEmbedder embedder) {
    _viewEmbedder = embedder;
  }


  @override
  ui.Paint createPaint() => SurfacePaint();

  @override
  ui.Vertices createVertices(
    ui.VertexMode mode,
    List<ui.Offset> positions, {
    List<ui.Offset>? textureCoordinates,
    List<ui.Color>? colors,
    List<int>? indices,
  }) => SurfaceVertices(
    mode,
    positions,
    colors: colors,
    indices: indices);

  @override
  ui.Vertices createVerticesRaw(
    ui.VertexMode mode,
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
  ui.Canvas createCanvas(ui.PictureRecorder recorder, [ui.Rect? cullRect]) =>
    SurfaceCanvas(recorder as EnginePictureRecorder, cullRect);

  @override
  ui.Gradient createLinearGradient(
    ui.Offset from,
    ui.Offset to,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    Float32List? matrix4
  ]) => GradientLinear(from, to, colors, colorStops, tileMode, matrix4);

  @override
  ui.Gradient createRadialGradient(
    ui.Offset center,
    double radius,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    Float32List? matrix4,
    ui.Offset? focal,
    double focalRadius = 0.0,
  ]) => GradientRadial(center, radius, colors, colorStops, tileMode, matrix4);

  @override
  ui.Gradient createConicalGradient(
    ui.Offset focal,
    double focalRadius,
    ui.Offset center,
    double radius,
    List<ui.Color> colors,
    [List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
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
  ui.Gradient createSweepGradient(
    ui.Offset center,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    double startAngle = 0.0,
    double endAngle = math.pi * 2,
    Float32List? matrix4
  ]) => GradientSweep(center, colors, colorStops, tileMode, startAngle, endAngle, matrix4);

  @override
  ui.PictureRecorder createPictureRecorder() => EnginePictureRecorder();

  @override
  ui.ImageFilter createBlurImageFilter({
    double sigmaX = 0.0,
    double sigmaY = 0.0,
    ui.TileMode tileMode = ui.TileMode.clamp
  }) => EngineImageFilter.blur(sigmaX: sigmaX, sigmaY: sigmaY, tileMode: tileMode);

  @override
  ui.ImageFilter createDilateImageFilter({ double radiusX = 0.0, double radiusY = 0.0}) {
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter createErodeImageFilter({ double radiusX = 0.0, double radiusY = 0.0}) {
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter createMatrixImageFilter(
    Float64List matrix4, {
    ui.FilterQuality filterQuality = ui.FilterQuality.low
  }) => EngineImageFilter.matrix(matrix: matrix4, filterQuality: filterQuality);

  @override
  ui.ImageFilter composeImageFilters({required ui.ImageFilter outer, required ui.ImageFilter inner}) {
    // ignore: avoid_unused_constructor_parameters
    throw UnimplementedError('ImageFilter.erode not implemented for HTML renderer.');
  }

  @override
  Future<ui.Codec> instantiateImageCodec(
    Uint8List list, {
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true}) async {
    final DomBlob blob = createDomBlob(<dynamic>[list.buffer]);
    return HtmlBlobCodec(blob);
  }

  @override
  Future<ui.Codec> instantiateImageCodecFromUrl(
    Uri uri, {
    WebOnlyImageCodecChunkCallback? chunkCallback}) {
      return futurize<ui.Codec>((Callback<ui.Codec> callback) {
        callback(HtmlCodec(uri.toString(), chunkCallback: chunkCallback));
        return null;
      });
  }

  @override
  void decodeImageFromPixels(
    Uint8List pixels,
    int width,
    int height,
    ui.PixelFormat format,
    ui.ImageDecoderCallback callback, {
    int? rowBytes,
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true
  }) {
    void executeCallback(ui.Codec codec) {
      codec.getNextFrame().then((ui.FrameInfo frameInfo) {
        callback(frameInfo.image);
      });
    }
    ui.createBmp(pixels, width, height, rowBytes ?? width, format).then(
      executeCallback);
  }

  @override
  ui.ImageShader createImageShader(
    ui.Image image,
    ui.TileMode tmx,
    ui.TileMode tmy,
    Float64List matrix4,
    ui.FilterQuality? filterQuality
  ) => EngineImageShader(image, tmx, tmy, matrix4, filterQuality);

  @override
  ui.Path createPath() => SurfacePath();

  @override
  ui.Path copyPath(ui.Path src) => SurfacePath.from(src as SurfacePath);

  @override
  ui.Path combinePaths(ui.PathOperation op, ui.Path path1, ui.Path path2) {
    throw UnimplementedError('combinePaths not implemented in HTML renderer.');
  }

  @override
  ui.TextStyle createTextStyle({
    ui.Color? color,
    ui.TextDecoration? decoration,
    ui.Color? decorationColor,
    ui.TextDecorationStyle? decorationStyle,
    double? decorationThickness,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
    ui.TextBaseline? textBaseline,
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? letterSpacing,
    double? wordSpacing,
    double? height,
    ui.TextLeadingDistribution? leadingDistribution,
    ui.Locale? locale,
    ui.Paint? background,
    ui.Paint? foreground,
    List<ui.Shadow>? shadows,
    List<ui.FontFeature>? fontFeatures,
    List<ui.FontVariation>? fontVariations
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
  ui.ParagraphStyle createParagraphStyle({
    ui.TextAlign? textAlign,
    ui.TextDirection? textDirection,
    int? maxLines,
    String? fontFamily,
    double? fontSize,
    double? height,
    ui.TextHeightBehavior? textHeightBehavior,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
    ui.StrutStyle? strutStyle,
    String? ellipsis,
    ui.Locale? locale
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
  ui.StrutStyle createStrutStyle({
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? height,
    ui.TextLeadingDistribution? leadingDistribution,
    double? leading,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
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
  ui.ParagraphBuilder createParagraphBuilder(ui.ParagraphStyle style) =>
    CanvasParagraphBuilder(style as EngineParagraphStyle);

  @override
  void clearFragmentProgramCache() { }

  @override
  Future<ui.FragmentProgram> createFragmentProgram(String assetKey) {
    return Future<HtmlFragmentProgram>.value(HtmlFragmentProgram());
  }
}
