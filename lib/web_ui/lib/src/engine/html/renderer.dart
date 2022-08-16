import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/renderer.dart';
import 'package:ui/ui.dart';

class HtmlRenderer implements Renderer {
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
}
