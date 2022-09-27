import 'dart:ffi';
import 'dart:typed_data';
import 'dart:wasm';

import 'package:ui/src/engine.dart' as engine;
import 'package:ui/ui.dart' as ui;
import 'raw/raw_memory.dart';
import 'raw/raw_path.dart';

enum PathDirection {
  clockwise,
  counterClockwise,
}

enum PathArcSize {
  small,
  large,
}

class SkwasmPath implements ui.Path {
  factory SkwasmPath() {
    return SkwasmPath._fromHandle(path_create());
  }

  factory SkwasmPath.from(SkwasmPath source) {
    return SkwasmPath._fromHandle(path_copy(source._handle));
  }

  SkwasmPath._fromHandle(this._handle);
  final PathHandle _handle;

  PathHandle get handle => _handle;

  void delete() {
    path_destroy(_handle);
  }

  @override
  ui.PathFillType get fillType {
    return ui.PathFillType.values[path_getFillType(_handle).toIntSigned()];
  }

  @override
  set fillType(ui.PathFillType fillType) {
    path_setFillType(_handle, fillType.index.toWasmI32());
  }

  @override
  void moveTo(double x, double y) {
    path_moveTo(_handle, x.toWasmF32(), y.toWasmF32());
  }

  @override
  void relativeMoveTo(double x, double y) {
    path_relativeMoveTo(_handle, x.toWasmF32(), y.toWasmF32());
  }

  @override
  void lineTo(double x, double y) {
    path_lineTo(_handle, x.toWasmF32(), y.toWasmF32());
  }

  @override
  void relativeLineTo(double x, double y) {
    path_relativeMoveTo(_handle, x.toWasmF32(), y.toWasmF32());
  }

  @override
  void quadraticBezierTo(double x1, double y1, double x2, double y2) {
    path_quadraticBezierTo(_handle, x1.toWasmF32(), y1.toWasmF32(),
        x2.toWasmF32(), y2.toWasmF32());
  }

  @override
  void relativeQuadraticBezierTo(double x1, double y1, double x2, double y2) {
    path_relativeQuadraticBezierTo(_handle, x1.toWasmF32(), y1.toWasmF32(),
        x2.toWasmF32(), y2.toWasmF32());
  }

  @override
  void cubicTo(
      double x1, double y1, double x2, double y2, double x3, double y3) {
    path_cubicTo(_handle, x1.toWasmF32(), y1.toWasmF32(), x2.toWasmF32(),
        y2.toWasmF32(), x3.toWasmF32(), y3.toWasmF32());
  }

  @override
  void relativeCubicTo(
      double x1, double y1, double x2, double y2, double x3, double y3) {
    path_relativeCubicTo(_handle, x1.toWasmF32(), y1.toWasmF32(),
        x2.toWasmF32(), y2.toWasmF32(), x3.toWasmF32(), y3.toWasmF32());
  }

  @override
  void conicTo(double x1, double y1, double x2, double y2, double w) {
    path_conicTo(_handle, x1.toWasmF32(), y1.toWasmF32(), x2.toWasmF32(),
        y2.toWasmF32(), w.toWasmF32());
  }

  @override
  void relativeConicTo(double x1, double y1, double x2, double y2, double w) {
    path_relativeConicTo(_handle, x1.toWasmF32(), y1.toWasmF32(),
        x2.toWasmF32(), y2.toWasmF32(), w.toWasmF32());
  }

  @override
  void arcTo(
      ui.Rect rect, double startAngle, double sweepAngle, bool forceMoveTo) {
    withStackScope((StackScope s) {
      final WasmI32 forceMoveToWasm =
          forceMoveTo ? 1.toWasmI32() : 0.toWasmI32();
      path_arcToOval(
          _handle,
          s.convertRect(rect),
          ui.toDegrees(startAngle).toWasmF32(),
          ui.toDegrees(sweepAngle).toWasmF32(),
          forceMoveToWasm);
    });
  }

  @override
  void arcToPoint(
    ui.Offset arcEnd, {
    ui.Radius radius = ui.Radius.zero,
    double rotation = 0.0,
    bool largeArc = false,
    bool clockwise = true,
  }) {
    final PathArcSize arcSize =
        largeArc ? PathArcSize.large : PathArcSize.small;
    final PathDirection pathDirection =
        clockwise ? PathDirection.clockwise : PathDirection.counterClockwise;
    path_arcToRotated(
        _handle,
        radius.x.toWasmF32(),
        radius.y.toWasmF32(),
        ui.toDegrees(rotation).toWasmF32(),
        arcSize.index.toWasmI32(),
        pathDirection.index.toWasmI32(),
        arcEnd.dx.toWasmF32(),
        arcEnd.dy.toWasmF32());
  }

  @override
  void relativeArcToPoint(
    ui.Offset arcEndDelta, {
    ui.Radius radius = ui.Radius.zero,
    double rotation = 0.0,
    bool largeArc = false,
    bool clockwise = true,
  }) {
    final PathArcSize arcSize =
        largeArc ? PathArcSize.large : PathArcSize.small;
    final PathDirection pathDirection =
        clockwise ? PathDirection.clockwise : PathDirection.counterClockwise;
    path_relativeArcToRotated(
        _handle,
        radius.x.toWasmF32(),
        radius.y.toWasmF32(),
        ui.toDegrees(rotation).toWasmF32(),
        arcSize.index.toWasmI32(),
        pathDirection.index.toWasmI32(),
        arcEndDelta.dx.toWasmF32(),
        arcEndDelta.dy.toWasmF32());
  }

  @override
  void addRect(ui.Rect rect) {
    withStackScope((StackScope s) {
      path_addRect(_handle, s.convertRect(rect));
    });
  }

  @override
  void addOval(ui.Rect rect) {
    withStackScope((StackScope s) {
      path_addOval(_handle, s.convertRect(rect));
    });
  }

  @override
  void addArc(ui.Rect rect, double startAngle, double sweepAngle) {
    withStackScope((StackScope s) {
      path_addArc(_handle, s.convertRect(rect),
          ui.toDegrees(startAngle).toWasmF32(), ui.toDegrees(sweepAngle).toWasmF32());
    });
  }

  @override
  void addPolygon(List<ui.Offset> points, bool close) {
    withStackScope((StackScope s) {
      final WasmI32 closeWasm = close ? 1.toWasmI32() : 0.toWasmI32();
      path_addPolygon(_handle, s.convertPointArray(points),
          points.length.toWasmI32(), closeWasm);
    });
  }

  @override
  void addRRect(ui.RRect rrect) {
    withStackScope((StackScope s) {
      path_addRRect(_handle, s.convertRRect(rrect));
    });
  }

  @override
  void addPath(ui.Path path, ui.Offset offset, {Float64List? matrix4}) {
    _addPath(path, offset, false, matrix4: matrix4);
  }

  @override
  void extendWithPath(ui.Path path, ui.Offset offset, {Float64List? matrix4}) {
    _addPath(path, offset, true, matrix4: matrix4);
  }

  void _addPath(ui.Path path, ui.Offset offset, bool extend, {Float64List? matrix4}) {
    assert(path is SkwasmPath);
    withStackScope((StackScope s) {
      final Pointer<Float> convertedMatrix =
          s.convertMatrix4toSkMatrix(matrix4 ?? engine.Matrix4.identity().toFloat64());
      convertedMatrix[2] += offset.dx;
      convertedMatrix[5] += offset.dy;
      final WasmI32 extendWasm = extend ? 1.toWasmI32() : 0.toWasmI32();
      path_addPath(_handle, (path as SkwasmPath)._handle, convertedMatrix, extendWasm);
    });
  }

  @override
  void close() {
    path_close(_handle);
  }

  @override
  void reset() {
    path_reset(_handle);
  }

  @override
  bool contains(ui.Offset point) {
    final WasmI32 result =
        path_contains(_handle, point.dx.toWasmF32(), point.dy.toWasmF32());
    return result.toIntSigned() != 0;
  }

  @override
  ui.Path shift(ui.Offset offset) {
    return transform(
        engine.Matrix4.translationValues(offset.dx, offset.dy, 0.0).toFloat64());
  }

  @override
  ui.Path transform(Float64List matrix4) {
    return withStackScope((StackScope s) {
      final PathHandle newPathHandle = path_copy(_handle);
      path_transform(newPathHandle, s.convertMatrix4toSkMatrix(matrix4));
      return SkwasmPath._fromHandle(newPathHandle);
    });
  }

  @override
  ui.Rect getBounds() {
    return withStackScope((StackScope s) {
      final Pointer<Float> rectBuffer = s.allocFloatArray(4);
      path_getBounds(_handle, rectBuffer);
      return ui.Rect.fromLTRB(
          rectBuffer[0], rectBuffer[1], rectBuffer[2], rectBuffer[3]);
    });
  }

  static SkwasmPath combine(ui.PathOperation operation, SkwasmPath path1, SkwasmPath path2) {
    return SkwasmPath._fromHandle(path_combine(
        operation.index.toWasmI32(), path1._handle, path2._handle));
  }
  
  @override
  ui.PathMetrics computeMetrics({bool forceClosed = false}) {
    // TODO: implement computeMetrics
    throw UnimplementedError();
  }
}
