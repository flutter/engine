import 'dart:async';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:meta/meta.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/fonts.dart';
import 'package:ui/ui.dart';

class CanvasKitRenderer implements Renderer {
  @override
  String get rendererTag => 'canvaskit';

  late final FontCollection _fontCollection = SkiaFontCollection();

  @override
  FontCollection get fontCollection => _fontCollection;

  /// The scene host, where the root canvas and overlay canvases are added to.
  DomElement? _sceneHost;
  DomElement? get sceneHost => _sceneHost;

  @visibleForTesting
  late Rasterizer rasterizer = Rasterizer();

  set resourceCacheMaxBytes(int bytes) => rasterizer.setSkiaResourceCacheMaxBytes(bytes);

  @override
  Future<void> initialize() async {
    if (windowFlutterCanvasKit != null) {
      canvasKit = windowFlutterCanvasKit!;
    } else if (useH5vccCanvasKit) {
      if (h5vcc?.canvasKit == null) {
        throw CanvasKitError('H5vcc CanvasKit implementation not found.');
      }
      canvasKit = h5vcc!.canvasKit!;
      windowFlutterCanvasKit = canvasKit;
    } else {
      canvasKit = await downloadCanvasKit();
      windowFlutterCanvasKit = canvasKit;
    }
  }

  String get canvasKitBuildUrl =>
    configuration.canvasKitBaseUrl + (kProfileMode ? 'profiling/' : '');
  String get canvasKitJavaScriptBindingsUrl =>
      '${canvasKitBuildUrl}canvaskit.js';
  String canvasKitWasmModuleUrl(String canvasKitBase, String file) =>
      canvasKitBase + file;

  /// Download and initialize the CanvasKit module.
  ///
  /// Downloads the CanvasKit JavaScript, then calls `CanvasKitInit` to download
  /// and intialize the CanvasKit wasm.
  @visibleForTesting
  Future<CanvasKit> downloadCanvasKit() async {
    await _downloadCanvasKitJs();
    final Completer<CanvasKit> canvasKitInitCompleter = Completer<CanvasKit>();
    final CanvasKitInitPromise canvasKitInitPromise =
        CanvasKitInit(CanvasKitInitOptions(
      locateFile: allowInterop((String file, String unusedBase) =>
          canvasKitWasmModuleUrl(canvasKitBuildUrl, file)),
    ));
    canvasKitInitPromise.then(allowInterop((CanvasKit ck) {
      canvasKitInitCompleter.complete(ck);
    }));
    return canvasKitInitCompleter.future;
  }

  /// Downloads the CanvasKit JavaScript file at [canvasKitBase].
  Future<void> _downloadCanvasKitJs() {
    final String canvasKitJavaScriptUrl = canvasKitJavaScriptBindingsUrl;

    final DomHTMLScriptElement canvasKitScript = createDomHTMLScriptElement();
    canvasKitScript.src = canvasKitJavaScriptUrl;

    final Completer<void> canvasKitLoadCompleter = Completer<void>();
    late DomEventListener callback;
    void loadEventHandler(DomEvent _) {
      canvasKitLoadCompleter.complete();
      canvasKitScript.removeEventListener('load', callback);
    }
    callback = allowInterop(loadEventHandler);
    canvasKitScript.addEventListener('load', callback);

    patchCanvasKitModule(canvasKitScript);

    return canvasKitLoadCompleter.future;
  }


  @override
  void reset() {
    /// CanvasKit uses a static scene element that never gets replaced, so it's
    /// added eagerly during initialization here and never touched, unless the
    /// system is reset due to hot restart or in a test.
    _sceneHost = createDomElement('flt-scene');
    flutterViewEmbedder.addSceneToSceneHost(_sceneHost);
  }

  @override
  Paint createPaint() => CkPaint();

  @override
  Vertices createVertices(
    VertexMode mode,
    List<Offset> positions, {
    List<Offset>? textureCoordinates,
    List<Color>? colors,
    List<int>? indices,
  }) => CkVertices(
    mode,
    positions,
    textureCoordinates: textureCoordinates,
    colors: colors,
    indices: indices);

  @override
  Vertices createVerticesRaw(
    VertexMode mode,
    Float32List positions, {
    Float32List? textureCoordinates,
    Int32List? colors,
    Uint16List? indices,
  }) => CkVertices.raw(
    mode,
    positions,
    textureCoordinates: textureCoordinates,
    colors: colors,
    indices: indices);

  @override
  Canvas createCanvas(PictureRecorder recorder, [Rect? cullRect]) =>
    CanvasKitCanvas(recorder, cullRect);

  @override
  Gradient createLinearGradient(
    Offset from,
    Offset to,
    List<Color> colors, [
    List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix4
  ]) => CkGradientLinear(from, to, colors, colorStops, tileMode, matrix4);

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
  ]) => CkGradientRadial(center, radius, colors, colorStops, tileMode, matrix4);

  @override
  Gradient createConicalGradient(
    Offset focal,
    double focalRadius,
    Offset center,
    double radius,
    List<Color> colors,
    [List<double>? colorStops,
    TileMode tileMode = TileMode.clamp,
    Float32List? matrix]
  ) => CkGradientConical(
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
  ]) => CkGradientSweep(center, colors, colorStops, tileMode, startAngle, endAngle, matrix4);

  @override
  PictureRecorder createPictureRecorder() => CkPictureRecorder();

  @override
  SceneBuilder createSceneBuilder() => LayerSceneBuilder();
    
  @override
  ImageFilter createBlurImageFilter({
    double sigmaX = 0.0, 
    double sigmaY = 0.0, 
    TileMode tileMode = TileMode.clamp
  }) => CkImageFilter.blur(sigmaX: sigmaX, sigmaY: sigmaY, tileMode: tileMode);
  
  @override
  ImageFilter createDilateImageFilter({double radiusX = 0.0, double radiusY = 0.0}) {
    // TODO(fzyzcjy): implement dilate. https://github.com/flutter/flutter/issues/101085
    throw UnimplementedError('ImageFilter.dilate not implemented for CanvasKit.');
  }
  
  @override
  ImageFilter createErodeImageFilter({double radiusX = 0.0, double radiusY = 0.0}) {
    // TODO(fzyzcjy): implement erode. https://github.com/flutter/flutter/issues/101085
    throw UnimplementedError('ImageFilter.erode not implemented for web platform.');
  }
  
  @override
  ImageFilter createMatrixImageFilter(
    Float64List matrix4, {
    FilterQuality filterQuality = FilterQuality.low
  }) => CkImageFilter.matrix(matrix: matrix4, filterQuality: filterQuality);

  @override
  ImageFilter composeImageFilters({required ImageFilter outer, required ImageFilter inner}) {
  // TODO(ferhat): add implementation
    throw UnimplementedError('ImageFilter.compose not implemented for CanvasKit.');
  }
  
  @override
  Future<Codec> instantiateImageCodec(
    Uint8List list, {
    int? targetWidth,
    int? targetHeight,
    bool allowUpscaling = true
  }) async => skiaInstantiateImageCodec(
    list,
    targetWidth,
    targetHeight);
    
  @override
  Future<Codec> instantiateImageCodecFromUrl(
    Uri uri, {
    WebOnlyImageCodecChunkCallback? chunkCallback
  }) => skiaInstantiateWebImageCodec(uri.toString(), chunkCallback);
  
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
  }) => skiaDecodeImageFromPixels(
    pixels,
    width,
    height,
    format,
    callback,
    rowBytes: rowBytes,
    targetWidth: targetWidth,
    targetHeight: targetHeight,
    allowUpscaling: allowUpscaling
  );
    
  @override
  ImageShader createImageShader(
    Image image,
    TileMode tmx,
    TileMode tmy,
    Float64List matrix4,
    FilterQuality? filterQuality
  ) => CkImageShader(image, tmx, tmy, matrix4, filterQuality);
  
  @override
  Path createPath() => CkPath();

  @override
  Path copyPath(Path src) => CkPath.from(src as CkPath);

  @override
  Path combinePaths(PathOperation op, Path path1, Path path2) =>
    CkPath.combine(op, path1, path2);
    
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
  }) => CkTextStyle(
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
    leadingDistribution: leadingDistribution,
    locale: locale,
    background: background as CkPaint?,
    foreground: foreground as CkPaint?,
    shadows: shadows,
    fontFeatures: fontFeatures,
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
  }) => CkParagraphStyle(
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
  }) => CkStrutStyle(
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
    CkParagraphBuilder(style);
    
  @override
  void renderScene(Scene scene) {
      // "Build finish" and "raster start" happen back-to-back because we
      // render on the same thread, so there's no overhead from hopping to
      // another thread.
      //
      // CanvasKit works differently from the HTML renderer in that in HTML
      // we update the DOM in SceneBuilder.build, which is these function calls
      // here are CanvasKit-only.
      frameTimingsOnBuildFinish();
      frameTimingsOnRasterStart();

      rasterizer.draw((scene as LayerScene).layerTree);
      frameTimingsOnRasterFinish();
  }
}
