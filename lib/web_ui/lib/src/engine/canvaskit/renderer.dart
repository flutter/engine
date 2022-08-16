import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/renderer.dart';
import 'package:ui/ui.dart';

class CanvasKitRenderer implements Renderer {
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
}
