// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:js_interop';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine/font_fallbacks.dart';
import 'package:ui/ui.dart' as ui;
import 'package:ui/ui_web/src/ui_web.dart' as ui_web;

import '../dom.dart';
import '../fonts.dart';
import '../renderer.dart';
import '../window.dart';
import 'external_renderer_definitions.dart';

final class ExternalRenderer implements Renderer {
  ExternalRenderer();

  final ExternalRendererDef _def = externalFlutterWebRenderer;

  @override
  void clearFragmentProgramCache() {
    _def.clearFragmentProgramCache();
  }

  @override
  ExternalPath combinePaths(ui.PathOperation op, ui.Path path1, ui.Path path2) {
    path1 as ExternalPath;
    path2 as ExternalPath;

    return ExternalPath(_def.combinePaths(
      op.toJS,
      path1._def,
      path2._def,
    ));
  }

  @override
  ExternalImageFilter composeImageFilters({
    required ui.ImageFilter outer,
    required ui.ImageFilter inner,
  }) {
    outer as ExternalImageFilter;
    inner as ExternalImageFilter;
    return ExternalImageFilter(
      _def.composeImageFilters(outer._def, inner._def),
    );
  }

  @override
  ExternalPath copyPath(ui.Path src) {
    src as ExternalPath;
    return ExternalPath(_def.copyPath(src._def));
  }

  @override
  ExternalImageFilter createBlurImageFilter({
    double sigmaX = 0.0,
    double sigmaY = 0.0,
    ui.TileMode tileMode = ui.TileMode.clamp,
  }) {
    return ExternalImageFilter(_def.createBlurImageFilter(
      sigmaX.toJS,
      sigmaY.toJS,
      tileMode.toJS,
    ));
  }

  @override
  ExternalCanvas createCanvas(ui.PictureRecorder recorder,
      [ui.Rect? cullRect]) {
    recorder as ExternalPictureRecorder;
    return ExternalCanvas(_def.createCanvas(
      recorder._def,
      cullRect?.toJS,
    ));
  }

  @override
  ExternalGradient createConicalGradient(
    ui.Offset focal,
    double focalRadius,
    ui.Offset center,
    double radius,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    Float32List? matrix,
  ]) {
    return ExternalGradient(
      _def.createConicalGradient(
        focal.toJS,
        focalRadius.toJS,
        center.toJS,
        radius.toJS,
        colors.toJS,
        colorStops?.toJSNumberArray,
        tileMode.toJS,
        matrix?.toJS,
      ),
    );
  }

  @override
  ExternalImageFilter createDilateImageFilter({
    double radiusX = 0.0,
    double radiusY = 0.0,
  }) {
    return ExternalImageFilter(
      _def.createDilateImageFilter(
        radiusX.toJS,
        radiusY.toJS,
      ),
    );
  }

  @override
  ExternalImageFilter createErodeImageFilter({
    double radiusX = 0.0,
    double radiusY = 0.0,
  }) {
    return ExternalImageFilter(
      _def.createErodeImageFilter(
        radiusX.toJS,
        radiusY.toJS,
      ),
    );
  }

  @override
  Future<ExternalFragmentProgram> createFragmentProgram(String assetKey) async {
    final jsProgram = await _def.createFragmentProgram(assetKey.toJS).toDart;
    return ExternalFragmentProgram(jsProgram);
  }

  @override
  FutureOr<ExternalImage> createImageFromImageBitmap(
      DomImageBitmap imageSource) async {
    final jsImage = await _def.createImageFromImageBitmap(imageSource).toDart;
    return ExternalImage(jsImage);
  }

  @override
  FutureOr<ExternalImage> createImageFromTextureSource(
    JSAny object, {
    required int width,
    required int height,
    required bool transferOwnership,
  }) async {
    final jsImage = await _def.createImageFromTextureSource(
      object,
      width.toJS,
      height.toJS,
      transferOwnership.toJS,
    ).toDart;
    return ExternalImage(jsImage);
  }

  @override
  ExternalImageShader createImageShader(
    ui.Image image,
    ui.TileMode tmx,
    ui.TileMode tmy,
    Float64List matrix4,
    ui.FilterQuality? filterQuality,
  ) {
    // TODO: implement createImageShader
    throw UnimplementedError();
  }

  @override
  ExternalLineMetrics createLineMetrics({
    required bool hardBreak,
    required double ascent,
    required double descent,
    required double unscaledAscent,
    required double height,
    required double width,
    required double left,
    required double baseline,
    required int lineNumber,
  }) {
    // TODO: implement createLineMetrics
    throw UnimplementedError();
  }

  @override
  ExternalGradient createLinearGradient(
    ui.Offset from,
    ui.Offset to,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    Float32List? matrix4,
  ]) {
    // TODO: implement createLinearGradient
    throw UnimplementedError();
  }

  @override
  ExternalImageFilter createMatrixImageFilter(Float64List matrix4,
      {ui.FilterQuality filterQuality = ui.FilterQuality.low}) {
    // TODO: implement createMatrixImageFilter
    throw UnimplementedError();
  }

  @override
  ExternalPaint createPaint() {
    // TODO: implement createPaint
    throw UnimplementedError();
  }

  @override
  ExternalParagraphBuilder createParagraphBuilder(ui.ParagraphStyle style) {
    // TODO: implement createParagraphBuilder
    throw UnimplementedError();
  }

  @override
  ExternalParagraphStyle createParagraphStyle({
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
    ui.Locale? locale,
  }) {
    // TODO: implement createParagraphStyle
    throw UnimplementedError();
  }

  @override
  ExternalPath createPath() {
    // TODO: implement createPath
    throw UnimplementedError();
  }

  @override
  ExternalPictureRecorder createPictureRecorder() {
    // TODO: implement createPictureRecorder
    throw UnimplementedError();
  }

  @override
  ExternalGradient createRadialGradient(
    ui.Offset center,
    double radius,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    Float32List? matrix4,
  ]) {
    // TODO: implement createRadialGradient
    throw UnimplementedError();
  }

  @override
  ExternalSceneBuilder createSceneBuilder() {
    // TODO: implement createSceneBuilder
    throw UnimplementedError();
  }

  @override
  ExternalStrutStyle createStrutStyle({
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? height,
    ui.TextLeadingDistribution? leadingDistribution,
    double? leading,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
    bool? forceStrutHeight,
  }) {
    // TODO: implement createStrutStyle
    throw UnimplementedError();
  }

  @override
  ExternalGradient createSweepGradient(
    ui.Offset center,
    List<ui.Color> colors, [
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    double startAngle = 0.0,
    double endAngle = math.pi * 2,
    Float32List? matrix4,
  ]) {
    // TODO: implement createSweepGradient
    throw UnimplementedError();
  }

  @override
  ExternalTextStyle createTextStyle({
    required ui.Color? color,
    required ui.TextDecoration? decoration,
    required ui.Color? decorationColor,
    required ui.TextDecorationStyle? decorationStyle,
    required double? decorationThickness,
    required ui.FontWeight? fontWeight,
    required ui.FontStyle? fontStyle,
    required ui.TextBaseline? textBaseline,
    required String? fontFamily,
    required List<String>? fontFamilyFallback,
    required double? fontSize,
    required double? letterSpacing,
    required double? wordSpacing,
    required double? height,
    required ui.TextLeadingDistribution? leadingDistribution,
    required ui.Locale? locale,
    required ui.Paint? background,
    required ui.Paint? foreground,
    required List<ui.Shadow>? shadows,
    required List<ui.FontFeature>? fontFeatures,
    required List<ui.FontVariation>? fontVariations,
  }) {
    // TODO: implement createTextStyle
    throw UnimplementedError();
  }

  @override
  ExternalVertices createVertices(
    ui.VertexMode mode,
    List<ui.Offset> positions, {
    List<ui.Offset>? textureCoordinates,
    List<ui.Color>? colors,
    List<int>? indices,
  }) {
    // TODO: implement createVertices
    throw UnimplementedError();
  }

  @override
  ExternalVertices createVerticesRaw(
    ui.VertexMode mode,
    Float32List positions, {
    Float32List? textureCoordinates,
    Int32List? colors,
    Uint16List? indices,
  }) {
    // TODO: implement createVerticesRaw
    throw UnimplementedError();
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
    bool allowUpscaling = true,
  }) {
    // TODO: implement decodeImageFromPixels
  }

  @override
  // TODO: implement fontCollection
  ExternalFlutterFontCollection get fontCollection =>
      throw UnimplementedError();

  @override
  FutureOr<void> initialize() {
    // TODO: implement initialize
    throw UnimplementedError();
  }

  @override
  Future<ExternalCodec> instantiateImageCodec(
    Uint8List list, {
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true,
  }) {
    // TODO: implement instantiateImageCodec
    throw UnimplementedError();
  }

  @override
  Future<ExternalCodec> instantiateImageCodecFromUrl(Uri uri,
      {ui_web.ImageCodecChunkCallback? chunkCallback}) {
    // TODO: implement instantiateImageCodecFromUrl
    throw UnimplementedError();
  }

  @override
  Future<void> renderScene(ui.Scene scene, EngineFlutterView view) {
    // TODO: implement renderScene
    throw UnimplementedError();
  }

  @override
  // TODO: implement rendererTag
  String get rendererTag => throw UnimplementedError();
}

class ExternalPath implements ui.Path {
  ExternalPath(this._def);

  final ExternalPathDef _def;

  @override
  ui.PathFillType get fillType {
    // TODO: implement fillType
    throw UnimplementedError();
  }

  set fillType(ui.PathFillType fillType) {
    // TODO: implement
  }

  @override
  void addArc(ui.Rect oval, double startAngle, double sweepAngle) {
    // TODO: implement addArc
  }

  @override
  void addOval(ui.Rect oval) {
    // TODO: implement addOval
  }

  @override
  void addPath(ui.Path path, ui.Offset offset, {Float64List? matrix4}) {
    // TODO: implement addPath
  }

  @override
  void addPolygon(List<ui.Offset> points, bool close) {
    // TODO: implement addPolygon
  }

  @override
  void addRRect(ui.RRect rrect) {
    // TODO: implement addRRect
  }

  @override
  void addRect(ui.Rect rect) {
    // TODO: implement addRect
  }

  @override
  void arcTo(
    ui.Rect rect,
    double startAngle,
    double sweepAngle,
    bool forceMoveTo,
  ) {
    // TODO: implement arcTo
  }

  @override
  void arcToPoint(
    ui.Offset arcEnd, {
    ui.Radius radius = ui.Radius.zero,
    double rotation = 0.0,
    bool largeArc = false,
    bool clockwise = true,
  }) {
    // TODO: implement arcToPoint
  }

  @override
  void close() {
    // TODO: implement close
  }

  @override
  ui.PathMetrics computeMetrics({bool forceClosed = false}) {
    // TODO: implement computeMetrics
    throw UnimplementedError();
  }

  @override
  void conicTo(double x1, double y1, double x2, double y2, double w) {
    // TODO: implement conicTo
  }

  @override
  bool contains(ui.Offset point) {
    // TODO: implement contains
    throw UnimplementedError();
  }

  @override
  void cubicTo(
    double x1,
    double y1,
    double x2,
    double y2,
    double x3,
    double y3,
  ) {
    // TODO: implement cubicTo
  }

  @override
  void extendWithPath(ui.Path path, ui.Offset offset, {Float64List? matrix4}) {
    // TODO: implement extendWithPath
  }

  @override
  ui.Rect getBounds() {
    // TODO: implement getBounds
    throw UnimplementedError();
  }

  @override
  void lineTo(double x, double y) {
    // TODO: implement lineTo
  }

  @override
  void moveTo(double x, double y) {
    // TODO: implement moveTo
  }

  @override
  void quadraticBezierTo(double x1, double y1, double x2, double y2) {
    // TODO: implement quadraticBezierTo
  }

  @override
  void relativeArcToPoint(
    ui.Offset arcEndDelta, {
    ui.Radius radius = ui.Radius.zero,
    double rotation = 0.0,
    bool largeArc = false,
    bool clockwise = true,
  }) {
    // TODO: implement relativeArcToPoint
  }

  @override
  void relativeConicTo(
    double x1,
    double y1,
    double x2,
    double y2,
    double w,
  ) {
    // TODO: implement relativeConicTo
  }

  @override
  void relativeCubicTo(
    double x1,
    double y1,
    double x2,
    double y2,
    double x3,
    double y3,
  ) {
    // TODO: implement relativeCubicTo
  }

  @override
  void relativeLineTo(double dx, double dy) {
    // TODO: implement relativeLineTo
  }

  @override
  void relativeMoveTo(double dx, double dy) {
    // TODO: implement relativeMoveTo
  }

  @override
  void relativeQuadraticBezierTo(
    double x1,
    double y1,
    double x2,
    double y2,
  ) {
    // TODO: implement relativeQuadraticBezierTo
  }

  @override
  void reset() {
    // TODO: implement reset
  }

  @override
  ui.Path shift(ui.Offset offset) {
    // TODO: implement shift
    throw UnimplementedError();
  }

  @override
  ui.Path transform(Float64List matrix4) {
    // TODO: implement transform
    throw UnimplementedError();
  }
}

class ExternalImageFilter implements ui.ImageFilter {
  ExternalImageFilter(this._def);

  final ExternalImageFilterDef _def;
}

class ExternalCanvas implements ui.Canvas {
  ExternalCanvas(this._def);

  final ExternalCanvasDef _def;

  @override
  void clipPath(ui.Path path, {bool doAntiAlias = true}) {
    // TODO: implement clipPath
  }

  @override
  void clipRRect(ui.RRect rrect, {bool doAntiAlias = true}) {
    // TODO: implement clipRRect
  }

  @override
  void clipRect(
    ui.Rect rect, {
    ui.ClipOp clipOp = ui.ClipOp.intersect,
    bool doAntiAlias = true,
  }) {
    // TODO: implement clipRect
  }

  @override
  void drawArc(
    ui.Rect rect,
    double startAngle,
    double sweepAngle,
    bool useCenter,
    ui.Paint paint,
  ) {
    // TODO: implement drawArc
  }

  @override
  void drawAtlas(
    ui.Image atlas,
    List<ui.RSTransform> transforms,
    List<ui.Rect> rects,
    List<ui.Color>? colors,
    ui.BlendMode? blendMode,
    ui.Rect? cullRect,
    ui.Paint paint,
  ) {
    // TODO: implement drawAtlas
  }

  @override
  void drawCircle(ui.Offset c, double radius, ui.Paint paint) {
    // TODO: implement drawCircle
  }

  @override
  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    // TODO: implement drawColor
  }

  @override
  void drawDRRect(ui.RRect outer, ui.RRect inner, ui.Paint paint) {
    // TODO: implement drawDRRect
  }

  @override
  void drawImage(ui.Image image, ui.Offset offset, ui.Paint paint) {
    // TODO: implement drawImage
  }

  @override
  void drawImageNine(
      ui.Image image, ui.Rect center, ui.Rect dst, ui.Paint paint) {
    // TODO: implement drawImageNine
  }

  @override
  void drawImageRect(ui.Image image, ui.Rect src, ui.Rect dst, ui.Paint paint) {
    // TODO: implement drawImageRect
  }

  @override
  void drawLine(ui.Offset p1, ui.Offset p2, ui.Paint paint) {
    // TODO: implement drawLine
  }

  @override
  void drawOval(ui.Rect rect, ui.Paint paint) {
    // TODO: implement drawOval
  }

  @override
  void drawPaint(ui.Paint paint) {
    // TODO: implement drawPaint
  }

  @override
  void drawParagraph(ui.Paragraph paragraph, ui.Offset offset) {
    // TODO: implement drawParagraph
  }

  @override
  void drawPath(ui.Path path, ui.Paint paint) {
    // TODO: implement drawPath
  }

  @override
  void drawPicture(ui.Picture picture) {
    // TODO: implement drawPicture
  }

  @override
  void drawPoints(
      ui.PointMode pointMode, List<ui.Offset> points, ui.Paint paint) {
    // TODO: implement drawPoints
  }

  @override
  void drawRRect(ui.RRect rrect, ui.Paint paint) {
    // TODO: implement drawRRect
  }

  @override
  void drawRawAtlas(
      ui.Image atlas,
      Float32List rstTransforms,
      Float32List rects,
      Int32List? colors,
      ui.BlendMode? blendMode,
      ui.Rect? cullRect,
      ui.Paint paint) {
    // TODO: implement drawRawAtlas
  }

  @override
  void drawRawPoints(
      ui.PointMode pointMode, Float32List points, ui.Paint paint) {
    // TODO: implement drawRawPoints
  }

  @override
  void drawRect(ui.Rect rect, ui.Paint paint) {
    // TODO: implement drawRect
  }

  @override
  void drawShadow(ui.Path path, ui.Color color, double elevation,
      bool transparentOccluder) {
    // TODO: implement drawShadow
  }

  @override
  void drawVertices(
      ui.Vertices vertices, ui.BlendMode blendMode, ui.Paint paint) {
    // TODO: implement drawVertices
  }

  @override
  ui.Rect getDestinationClipBounds() {
    // TODO: implement getDestinationClipBounds
    throw UnimplementedError();
  }

  @override
  ui.Rect getLocalClipBounds() {
    // TODO: implement getLocalClipBounds
    throw UnimplementedError();
  }

  @override
  int getSaveCount() {
    // TODO: implement getSaveCount
    throw UnimplementedError();
  }

  @override
  Float64List getTransform() {
    // TODO: implement getTransform
    throw UnimplementedError();
  }

  @override
  void restore() {
    // TODO: implement restore
  }

  @override
  void restoreToCount(int count) {
    // TODO: implement restoreToCount
  }

  @override
  void rotate(double radians) {
    // TODO: implement rotate
  }

  @override
  void save() {
    // TODO: implement save
  }

  @override
  void saveLayer(ui.Rect? bounds, ui.Paint paint) {
    // TODO: implement saveLayer
  }

  @override
  void scale(double sx, [double? sy]) {
    // TODO: implement scale
  }

  @override
  void skew(double sx, double sy) {
    // TODO: implement skew
  }

  @override
  void transform(Float64List matrix4) {
    // TODO: implement transform
  }

  @override
  void translate(double dx, double dy) {
    // TODO: implement translate
  }
}

class ExternalGradient implements ui.Gradient {
  ExternalGradient(this._def);

  final ExternalGradientDef _def;

  @override
  // TODO: implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();

  @override
  void dispose() {
    // TODO: implement dispose
  }
}

class ExternalFragmentProgram implements ui.FragmentProgram {
  ExternalFragmentProgram(this._def);

  final ExternalFragmentProgramDef _def;
  @override
  ui.FragmentShader fragmentShader() {
    // TODO: implement fragmentShader
    throw UnimplementedError();
  }
}

class ExternalImage implements ui.Image {
  ExternalImage(this._def);

  final ExternalImageDef _def;

  @override
  ui.Image clone() {
    // TODO: implement clone
    throw UnimplementedError();
  }

  @override
  // TODO: implement colorSpace
  ui.ColorSpace get colorSpace => throw UnimplementedError();

  @override
  // TODO: implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();

  @override
  List<StackTrace>? debugGetOpenHandleStackTraces() {
    // TODO: implement debugGetOpenHandleStackTraces
    throw UnimplementedError();
  }

  @override
  void dispose() {
    // TODO: implement dispose
  }

  @override
  // TODO: implement height
  int get height => throw UnimplementedError();

  @override
  bool isCloneOf(ui.Image other) {
    // TODO: implement isCloneOf
    throw UnimplementedError();
  }

  @override
  Future<ByteData?> toByteData({
    ui.ImageByteFormat format = ui.ImageByteFormat.rawRgba,
  }) {
    // TODO: implement toByteData
    throw UnimplementedError();
  }

  @override
  // TODO: implement width
  int get width => throw UnimplementedError();
}

class ExternalImageShader implements ui.ImageShader {
  ExternalImageShader(this._def);

  final ExternalImageShaderDef _def;

  @override
  // TODO: implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();

  @override
  void dispose() {
    // TODO: implement dispose
  }
}

class ExternalLineMetrics implements ui.LineMetrics {
  ExternalLineMetrics(this._def);

  final ExternalLineMetricsDef _def;

  @override
  // TODO: implement ascent
  double get ascent => throw UnimplementedError();

  @override
  // TODO: implement baseline
  double get baseline => throw UnimplementedError();

  @override
  // TODO: implement descent
  double get descent => throw UnimplementedError();

  @override
  // TODO: implement hardBreak
  bool get hardBreak => throw UnimplementedError();

  @override
  // TODO: implement height
  double get height => throw UnimplementedError();

  @override
  // TODO: implement left
  double get left => throw UnimplementedError();

  @override
  // TODO: implement lineNumber
  int get lineNumber => throw UnimplementedError();

  @override
  // TODO: implement unscaledAscent
  double get unscaledAscent => throw UnimplementedError();

  @override
  // TODO: implement width
  double get width => throw UnimplementedError();
}

class ExternalPaint implements ui.Paint {
  ExternalPaint(this._def);

  final ExternalPaintDef _def;

  @override
  ui.BlendMode get blendMode {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set blendMode(ui.BlendMode value) {
    // TODO: implement
  }

  @override
  ui.Color get color {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set color(ui.Color value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.ColorFilter? get colorFilter {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set colorFilter(ui.ColorFilter? value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.FilterQuality get filterQuality {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set filterQuality(ui.FilterQuality value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.ImageFilter? get imageFilter {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set imageFilter(ui.ImageFilter? value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  bool get invertColors {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set invertColors(bool value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  bool get isAntiAlias {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set isAntiAlias(bool value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.MaskFilter? get maskFilter {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set maskFilter(ui.MaskFilter? value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.Shader? get shader {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set shader(ui.Shader? value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.StrokeCap get strokeCap {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set strokeCap(ui.StrokeCap value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.StrokeJoin get strokeJoin {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set strokeJoin(ui.StrokeJoin value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  double get strokeMiterLimit {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set strokeMiterLimit(double value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  double get strokeWidth {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set strokeWidth(double value) {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  ui.PaintingStyle get style {
    // TODO: implement
    throw UnimplementedError();
  }

  @override
  set style(ui.PaintingStyle value) {
    // TODO: implement
    throw UnimplementedError();
  }
}

class ExternalParagraphBuilder implements ui.ParagraphBuilder {
  ExternalParagraphBuilder(this._def);

  final ExternalParagraphBuilderDef _def;

  @override
  void addPlaceholder(
    double width,
    double height,
    ui.PlaceholderAlignment alignment, {
    double scale = 1.0,
    double? baselineOffset,
    ui.TextBaseline? baseline,
  }) {
    // TODO: implement addPlaceholder
  }

  @override
  void addText(String text) {
    // TODO: implement addText
  }

  @override
  ExternalParagraph build() {
    // TODO: implement build
    throw UnimplementedError();
  }

  @override
  // TODO: implement placeholderCount
  int get placeholderCount => throw UnimplementedError();

  @override
  // TODO: implement placeholderScales
  List<double> get placeholderScales => throw UnimplementedError();

  @override
  void pop() {
    // TODO: implement pop
  }

  @override
  void pushStyle(ui.TextStyle style) {
    // TODO: implement pushStyle
  }
}

class ExternalParagraph implements ui.Paragraph {
  ExternalParagraph(this._def);

  final ExternalParagraphDef _def;

  @override
  // TODO: implement alphabeticBaseline
  double get alphabeticBaseline => throw UnimplementedError();

  @override
  List<ExternalLineMetrics> computeLineMetrics() {
    // TODO: implement computeLineMetrics
    throw UnimplementedError();
  }

  @override
  // TODO: implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();

  @override
  // TODO: implement didExceedMaxLines
  bool get didExceedMaxLines => throw UnimplementedError();

  @override
  void dispose() {
    // TODO: implement dispose
  }

  @override
  List<ui.TextBox> getBoxesForPlaceholders() {
    // TODO: implement getBoxesForPlaceholders
    throw UnimplementedError();
  }

  @override
  List<ui.TextBox> getBoxesForRange(
    int start,
    int end, {
    ui.BoxHeightStyle boxHeightStyle = ui.BoxHeightStyle.tight,
    ui.BoxWidthStyle boxWidthStyle = ui.BoxWidthStyle.tight,
  }) {
    // TODO: implement getBoxesForRange
    throw UnimplementedError();
  }

  @override
  ui.GlyphInfo? getClosestGlyphInfoForOffset(ui.Offset offset) {
    // TODO: implement getClosestGlyphInfoForOffset
    throw UnimplementedError();
  }

  @override
  ui.GlyphInfo? getGlyphInfoAt(int codeUnitOffset) {
    // TODO: implement getGlyphInfoAt
    throw UnimplementedError();
  }

  @override
  ui.TextRange getLineBoundary(ui.TextPosition position) {
    // TODO: implement getLineBoundary
    throw UnimplementedError();
  }

  @override
  ExternalLineMetrics? getLineMetricsAt(int lineNumber) {
    // TODO: implement getLineMetricsAt
    throw UnimplementedError();
  }

  @override
  int? getLineNumberAt(int codeUnitOffset) {
    // TODO: implement getLineNumberAt
    throw UnimplementedError();
  }

  @override
  ui.TextPosition getPositionForOffset(ui.Offset offset) {
    // TODO: implement getPositionForOffset
    throw UnimplementedError();
  }

  @override
  ui.TextRange getWordBoundary(ui.TextPosition position) {
    // TODO: implement getWordBoundary
    throw UnimplementedError();
  }

  @override
  // TODO: implement height
  double get height => throw UnimplementedError();

  @override
  // TODO: implement ideographicBaseline
  double get ideographicBaseline => throw UnimplementedError();

  @override
  void layout(ui.ParagraphConstraints constraints) {
    // TODO: implement layout
  }

  @override
  // TODO: implement longestLine
  double get longestLine => throw UnimplementedError();

  @override
  // TODO: implement maxIntrinsicWidth
  double get maxIntrinsicWidth => throw UnimplementedError();

  @override
  // TODO: implement minIntrinsicWidth
  double get minIntrinsicWidth => throw UnimplementedError();

  @override
  // TODO: implement numberOfLines
  int get numberOfLines => throw UnimplementedError();

  @override
  // TODO: implement width
  double get width => throw UnimplementedError();
}

class ExternalParagraphStyle implements ui.ParagraphStyle {
  ExternalParagraphStyle(this._def);

  final ExternalParagraphStyleDef _def;
}

class ExternalPictureRecorder implements ui.PictureRecorder {
  ExternalPictureRecorder(this._def);

  final ExternalPictureRecorderDef _def;

  @override
  ui.Picture endRecording() {
    // TODO: implement endRecording
    throw UnimplementedError();
  }

  @override
  // TODO: implement isRecording
  bool get isRecording => throw UnimplementedError();
}

class ExternalSceneBuilder implements ui.SceneBuilder {
  ExternalSceneBuilder(this._def);

  final ExternalSceneBuilderDef _def;

  @override
  void addPerformanceOverlay(int enabledOptions, ui.Rect bounds) {
    // TODO: implement addPerformanceOverlay
  }

  @override
  void addPicture(
    ui.Offset offset,
    ui.Picture picture, {
    bool isComplexHint = false,
    bool willChangeHint = false,
  }) {
    // TODO: implement addPicture
  }

  @override
  void addPlatformView(
    int viewId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0,
  }) {
    // TODO: implement addPlatformView
  }

  @override
  void addRetained(ui.EngineLayer retainedLayer) {
    // TODO: implement addRetained
  }

  @override
  void addTexture(
    int textureId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0,
    bool freeze = false,
    ui.FilterQuality filterQuality = ui.FilterQuality.low,
  }) {
    // TODO: implement addTexture
  }

  @override
  ExternalScene build() {
    // TODO: implement build
    throw UnimplementedError();
  }

  @override
  void pop() {
    // TODO: implement pop
  }

  @override
  ExternalBackdropFilterEngineLayer pushBackdropFilter(
    ui.ImageFilter filter, {
    ui.BlendMode blendMode = ui.BlendMode.srcOver,
    ui.BackdropFilterEngineLayer? oldLayer,
    int? backdropId,
  }) {
    // TODO: implement pushBackdropFilter
    throw UnimplementedError();
  }

  @override
  ExternalClipPathEngineLayer pushClipPath(
    ui.Path path, {
    ui.Clip clipBehavior = ui.Clip.antiAlias,
    ui.ClipPathEngineLayer? oldLayer,
  }) {
    // TODO: implement pushClipPath
    throw UnimplementedError();
  }

  @override
  ExternalClipRRectEngineLayer pushClipRRect(
    ui.RRect rrect, {
    required ui.Clip clipBehavior,
    ui.ClipRRectEngineLayer? oldLayer,
  }) {
    // TODO: implement pushClipRRect
    throw UnimplementedError();
  }

  @override
  ExternalClipRectEngineLayer pushClipRect(
    ui.Rect rect, {
    ui.Clip clipBehavior = ui.Clip.antiAlias,
    ui.ClipRectEngineLayer? oldLayer,
  }) {
    // TODO: implement pushClipRect
    throw UnimplementedError();
  }

  @override
  ExternalColorFilterEngineLayer pushColorFilter(
    ui.ColorFilter filter, {
    ui.ColorFilterEngineLayer? oldLayer,
  }) {
    // TODO: implement pushColorFilter
    throw UnimplementedError();
  }

  @override
  ExternalImageFilterEngineLayer pushImageFilter(
    ui.ImageFilter filter, {
    ui.Offset offset = ui.Offset.zero,
    ui.ImageFilterEngineLayer? oldLayer,
  }) {
    // TODO: implement pushImageFilter
    throw UnimplementedError();
  }

  @override
  ExternalOffsetEngineLayer pushOffset(
    double dx,
    double dy, {
    ui.OffsetEngineLayer? oldLayer,
  }) {
    // TODO: implement pushOffset
    throw UnimplementedError();
  }

  @override
  ExternalOpacityEngineLayer pushOpacity(
    int alpha, {
    ui.Offset offset = ui.Offset.zero,
    ui.OpacityEngineLayer? oldLayer,
  }) {
    // TODO: implement pushOpacity
    throw UnimplementedError();
  }

  @override
  ExternalShaderMaskEngineLayer pushShaderMask(
    ui.Shader shader,
    ui.Rect maskRect,
    ui.BlendMode blendMode, {
    ui.ShaderMaskEngineLayer? oldLayer,
    ui.FilterQuality filterQuality = ui.FilterQuality.low,
  }) {
    // TODO: implement pushShaderMask
    throw UnimplementedError();
  }

  @override
  ExternalTransformEngineLayer pushTransform(
    Float64List matrix4, {
    ui.TransformEngineLayer? oldLayer,
  }) {
    // TODO: implement pushTransform
    throw UnimplementedError();
  }

  @override
  void setProperties(
    double width,
    double height,
    double insetTop,
    double insetRight,
    double insetBottom,
    double insetLeft,
    bool focusable,
  ) {
    // TODO: implement setProperties
  }
}

class ExternalScene implements ui.Scene {
  ExternalScene(this._def);

  final ExternalScene _def;

  @override
  void dispose() {
    // TODO: implement dispose
  }

  @override
  Future<ExternalImage> toImage(int width, int height) {
    // TODO: implement toImage
    throw UnimplementedError();
  }

  @override
  ExternalImage toImageSync(int width, int height) {
    // TODO: implement toImageSync
    throw UnimplementedError();
  }
}

class ExternalBackdropFilterEngineLayer
    implements ui.BackdropFilterEngineLayer {
  ExternalBackdropFilterEngineLayer(this._def);

  final ExternalBackdropFilterEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalClipPathEngineLayer implements ui.ClipPathEngineLayer {
  ExternalClipPathEngineLayer(this._def);

  final ExternalClipPathEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalClipRRectEngineLayer implements ui.ClipRRectEngineLayer {
  ExternalClipRRectEngineLayer(this._def);

  final ExternalClipRRectEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalClipRectEngineLayer implements ui.ClipRectEngineLayer {
  ExternalClipRectEngineLayer(this._def);

  final ExternalClipRectEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalColorFilterEngineLayer implements ui.ColorFilterEngineLayer {
  ExternalColorFilterEngineLayer(this._def);

  final ExternalColorFilterEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalImageFilterEngineLayer implements ui.ImageFilterEngineLayer {
  ExternalImageFilterEngineLayer(this._def);

  final ExternalImageFilterEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalOffsetEngineLayer implements ui.OffsetEngineLayer {
  ExternalOffsetEngineLayer(this._def);

  final ExternalOffsetEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalOpacityEngineLayer implements ui.OpacityEngineLayer {
  ExternalOpacityEngineLayer(this._def);

  final ExternalOpacityEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalShaderMaskEngineLayer implements ui.ShaderMaskEngineLayer {
  ExternalShaderMaskEngineLayer(this._def);

  final ExternalShaderMaskEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalTransformEngineLayer implements ui.TransformEngineLayer {
  ExternalTransformEngineLayer(this._def);

  final ExternalTransformEngineLayerDef _def;

  @override
  void dispose() {
    _def.dispose();
  }
}

class ExternalStrutStyle implements ui.StrutStyle {}

class ExternalTextStyle implements ui.TextStyle {}

class ExternalVertices implements ui.Vertices {
  @override
  // TODO: implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();

  @override
  void dispose() {
    // TODO: implement dispose
  }
}

class ExternalFlutterFontCollection implements FlutterFontCollection {
  @override
  void clear() {
    // TODO: implement clear
  }

  @override
  void debugResetFallbackFonts() {
    // TODO: implement debugResetFallbackFonts
  }

  @override
  // TODO: implement fontFallbackManager
  FontFallbackManager? get fontFallbackManager => throw UnimplementedError();

  @override
  Future<AssetFontsResult> loadAssetFonts(FontManifest manifest) {
    // TODO: implement loadAssetFonts
    throw UnimplementedError();
  }

  @override
  Future<bool> loadFontFromList(Uint8List list, {String? fontFamily}) {
    // TODO: implement loadFontFromList
    throw UnimplementedError();
  }
}

class ExternalCodec implements ui.Codec {
  @override
  void dispose() {
    // TODO: implement dispose
  }

  @override
  // TODO: implement frameCount
  int get frameCount => throw UnimplementedError();

  @override
  Future<ui.FrameInfo> getNextFrame() {
    // TODO: implement getNextFrame
    throw UnimplementedError();
  }

  @override
  // TODO: implement repetitionCount
  int get repetitionCount => throw UnimplementedError();
}
