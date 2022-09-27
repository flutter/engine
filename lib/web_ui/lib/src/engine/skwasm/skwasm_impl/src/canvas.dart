import 'dart:ffi';
import 'dart:typed_data';
import 'dart:wasm';

import 'package:ui/ui.dart' as ui;

import 'image.dart';
import 'paint.dart';
import 'paragraph.dart';
import 'path.dart';
import 'picture.dart';
import 'raw/raw_canvas.dart';
import 'raw/raw_memory.dart';
import 'raw/raw_picture.dart';
import 'vertices.dart';

class SkwasmCanvas implements ui.Canvas {
  factory SkwasmCanvas(PictureRecorder recorder, [ui.Rect cullRect = ui.Rect.largest]) {
    return SkwasmCanvas.fromHandle(withStackScope((StackScope s) {
      return pictureRecorder_beginRecording(
          recorder.handle, s.convertRect(cullRect));
    }));
  }

  SkwasmCanvas.fromHandle(this._handle);
  CanvasHandle _handle;

  void delete() {
    canvas_destroy(_handle);
  }

  void save() {
    canvas_save(_handle);
  }

  void saveLayer(ui.Rect? bounds, Paint paint) {
    if (bounds != null) {
      withStackScope((StackScope s) {
        canvas_saveLayer(_handle, s.convertRect(bounds), paint.handle);
      });
    } else {
      canvas_saveLayer(_handle, nullptr, paint.handle);
    }
  }

  void restore() {
    canvas_restore(_handle);
  }

  int getSaveCount() {
    return canvas_getSaveCount(_handle).toIntSigned();
  }

  void translate(double dx, double dy) {
    canvas_translate(_handle, dx.toWasmF32(), dy.toWasmF32());
  }

  void scale(double sx, [double? sy]) {
    canvas_scale(_handle, sx.toWasmF32(), (sy ?? sx).toWasmF32());
  }

  void rotate(double radians) {
    canvas_rotate(_handle, ui.toDegrees(radians).toWasmF32());
  }

  void skew(double sx, double sy) {
    canvas_skew(_handle, sx.toWasmF32(), sy.toWasmF32());
  }

  void transform(Float64List matrix4) {
    withStackScope((StackScope s) {
      canvas_transform(_handle, s.convertMatrix4toSkM44(matrix4));
    });
  }

  void clipRect(ui.Rect rect,
      {ui.ClipOp clipOp = ui.ClipOp.intersect, bool doAntiAlias = true}) {
    withStackScope((StackScope s) {
      canvas_clipRect(_handle, s.convertRect(rect), clipOp.index.toWasmI32(),
          doAntiAlias.toWasmI32());
    });
  }

  void clipRRect(ui.RRect rrect, {bool doAntialias = true}) {
    withStackScope((StackScope s) {
      canvas_clipRRect(_handle, s.convertRRect(rrect), doAntialias.toWasmI32());
    });
  }

  void clipPath(Path path, {bool doAntiAlias = true}) {
    canvas_clipPath(_handle, path.handle, doAntiAlias.toWasmI32());
  }

  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    canvas_drawColor(
        _handle, color.value.toWasmI32(), blendMode.index.toWasmI32());
  }

  void drawLine(ui.Offset p1, ui.Offset p2, Paint paint) {
    canvas_drawLine(_handle, p1.dx.toWasmF32(), p1.dy.toWasmF32(),
        p2.dx.toWasmF32(), p2.dy.toWasmF32(), paint.handle);
  }

  void drawPaint(Paint paint) {
    canvas_drawPaint(_handle, paint.handle);
  }

  void drawRect(ui.Rect rect, Paint paint) {
    withStackScope((StackScope s) {
      canvas_drawRect(_handle, s.convertRect(rect), paint.handle);
    });
  }

  void drawRRect(ui.RRect rrect, Paint paint) {
    withStackScope((StackScope s) {
      canvas_drawRRect(_handle, s.convertRRect(rrect), paint.handle);
    });
  }

  void drawDRRect(ui.RRect outer, ui.RRect inner, Paint paint) {
    withStackScope((StackScope s) {
      canvas_drawDRRect(
          _handle, s.convertRRect(outer), s.convertRRect(inner), paint.handle);
    });
  }

  void drawOval(ui.Rect rect, Paint paint) {
    withStackScope((StackScope s) {
      canvas_drawOval(_handle, s.convertRect(rect), paint.handle);
    });
  }

  void drawCircle(ui.Offset center, double radius, Paint paint) {
    canvas_drawCircle(_handle, center.dx.toWasmF32(), center.dy.toWasmF32(),
        radius.toWasmF32(), paint.handle);
  }

  void drawArc(ui.Rect rect, double startAngle, double sweepAngle, bool useCenter,
      Paint paint) {
    withStackScope((StackScope s) {
      canvas_drawArc(
          _handle,
          s.convertRect(rect),
          ui.toDegrees(startAngle).toWasmF32(),
          ui.toDegrees(sweepAngle).toWasmF32(),
          useCenter.toWasmI32(),
          paint.handle);
    });
  }

  void drawPath(Path path, Paint paint) {
    canvas_drawPath(_handle, path.handle, paint.handle);
  }

  void drawImage(Image image, ui.Offset offset, Paint paint) {
    throw UnimplementedError();
  }

  void drawImageRect(Image image, ui.Rect src, ui.Rect dst, Paint paint) {
    throw UnimplementedError();
  }

  void drawImageNine(Image image, ui.Rect center, ui.Rect dst, Paint paint) {
    throw UnimplementedError();
  }

  void drawPicture(Picture picture) {
    canvas_drawPicture(_handle, picture.handle);
  }

  void drawParagraph(Paragraph paragraph, ui.Offset offset) {
    throw UnimplementedError();
  }

  void drawPoints(ui.PointMode pointMode, List<ui.Offset> points, Paint paint) {
    throw UnimplementedError();
  }

  void drawRawPoints(ui.PointMode pointMode, Float32List points, Paint paint) {
    throw UnimplementedError();
  }

  void drawVertices(Vertices vertices, ui.BlendMode blendMode, Paint paint) {
    throw UnimplementedError();
  }

  void drawAtlas(
    Image atlas,
    List<ui.RSTransform> transforms,
    List<ui.Rect> rects,
    List<ui.Color>? colors,
    ui.BlendMode? blendMode,
    ui.Rect? cullRect,
    Paint paint,
  ) {
    throw UnimplementedError();
  }

  void drawRawAtlas(
    Image atlas,
    Float32List rstTransforms,
    Float32List rects,
    Int32List? colors,
    ui.BlendMode? blendMode,
    ui.Rect? cullRect,
    Paint paint,
  ) {
    throw UnimplementedError();
  }

  void drawShadow(
    Path path,
    ui.Color color,
    double elevation,
    bool transparentOccluder,
  ) {
    throw UnimplementedError();
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
  Float64List getTransform() {
    // TODO: implement getTransform
    throw UnimplementedError();
  }
  
  @override
  void restoreToCount(int count) {
    // TODO: implement restoreToCount
  }
}
