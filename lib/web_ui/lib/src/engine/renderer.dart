import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine/browser_detection.dart';
import 'package:ui/src/engine/canvaskit/renderer.dart';
import 'package:ui/src/engine/configuration.dart';
import 'package:ui/src/engine/html/renderer.dart';
import 'package:ui/ui.dart';

late final Renderer _renderer = Renderer._internal();
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
}
