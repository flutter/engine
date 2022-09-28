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
  factory SkwasmCanvas(SkwasmPictureRecorder recorder, [ui.Rect cullRect = ui.Rect.largest]) {
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

  @override
  void save() {
    canvas_save(_handle);
  }

  @override
  void saveLayer(ui.Rect? bounds, ui.Paint uiPaint) {
    assert(uiPaint is SkwasmPaint);
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    if (bounds != null) {
      withStackScope((StackScope s) {
        canvas_saveLayer(_handle, s.convertRect(bounds), paint.handle);
      });
    } else {
      canvas_saveLayer(_handle, nullptr, paint.handle);
    }
  }

  @override
  void restore() {
    canvas_restore(_handle);
  }

  @override
  int getSaveCount() {
    return canvas_getSaveCount(_handle).toIntSigned();
  }

  @override
  void translate(double dx, double dy) {
    canvas_translate(_handle, dx.toWasmF32(), dy.toWasmF32());
  }

  @override
  void scale(double sx, [double? sy]) {
    canvas_scale(_handle, sx.toWasmF32(), (sy ?? sx).toWasmF32());
  }

  @override
  void rotate(double radians) {
    canvas_rotate(_handle, ui.toDegrees(radians).toWasmF32());
  }

  @override
  void skew(double sx, double sy) {
    canvas_skew(_handle, sx.toWasmF32(), sy.toWasmF32());
  }

  @override
  void transform(Float64List matrix4) {
    withStackScope((StackScope s) {
      canvas_transform(_handle, s.convertMatrix4toSkM44(matrix4));
    });
  }

  @override
  void clipRect(ui.Rect rect,
      {ui.ClipOp clipOp = ui.ClipOp.intersect, bool doAntiAlias = true}) {
    withStackScope((StackScope s) {
      canvas_clipRect(_handle, s.convertRect(rect), clipOp.index.toWasmI32(),
          doAntiAlias.toWasmI32());
    });
  }

  @override
  void clipRRect(ui.RRect rrect, {bool doAntialias = true}) {
    withStackScope((StackScope s) {
      canvas_clipRRect(_handle, s.convertRRect(rrect), doAntialias.toWasmI32());
    });
  }

  @override
  void clipPath(ui.Path uiPath, {bool doAntiAlias = true}) {
    assert(uiPath is SkwasmPath);
    final SkwasmPath path = uiPath as SkwasmPath;
    canvas_clipPath(_handle, path.handle, doAntiAlias.toWasmI32());
  }

  @override
  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    canvas_drawColor(
        _handle, color.value.toWasmI32(), blendMode.index.toWasmI32());
  }

  @override
  void drawLine(ui.Offset p1, ui.Offset p2, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    canvas_drawLine(_handle, p1.dx.toWasmF32(), p1.dy.toWasmF32(),
        p2.dx.toWasmF32(), p2.dy.toWasmF32(), paint.handle);
  }

  @override
  void drawPaint(ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    canvas_drawPaint(_handle, paint.handle);
  }

  @override
  void drawRect(ui.Rect rect, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvas_drawRect(_handle, s.convertRect(rect), paint.handle);
    });
  }

  @override
  void drawRRect(ui.RRect rrect, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvas_drawRRect(_handle, s.convertRRect(rrect), paint.handle);
    });
  }

  @override
  void drawDRRect(ui.RRect outer, ui.RRect inner, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvas_drawDRRect(
          _handle, s.convertRRect(outer), s.convertRRect(inner), paint.handle);
    });
  }

  @override
  void drawOval(ui.Rect rect, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvas_drawOval(_handle, s.convertRect(rect), paint.handle);
    });
  }

  @override
  void drawCircle(ui.Offset center, double radius, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    canvas_drawCircle(_handle, center.dx.toWasmF32(), center.dy.toWasmF32(),
        radius.toWasmF32(), paint.handle);
  }

  @override
  void drawArc(ui.Rect rect, double startAngle, double sweepAngle, bool useCenter,
      ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
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

  @override
  void drawPath(ui.Path uiPath, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    final SkwasmPath path = uiPath as SkwasmPath;
    canvas_drawPath(_handle, path.handle, paint.handle);
  }

  @override
  void drawImage(ui.Image uiImage, ui.Offset offset, ui.Paint uiPaint) {
    throw UnimplementedError();
  }

  @override
  void drawImageRect(ui.Image uiImage, ui.Rect src, ui.Rect dst, ui.Paint uiPaint) {
    throw UnimplementedError();
  }

  @override
  void drawImageNine(ui.Image uiImage, ui.Rect center, ui.Rect dst, ui.Paint uiPaint) {
    throw UnimplementedError();
  }

  @override
  void drawPicture(ui.Picture picture) {
    canvas_drawPicture(_handle, (picture as SkwasmPicture).handle);
  }

  @override
  void drawParagraph(ui.Paragraph uiParagraph, ui.Offset offset) {
    throw UnimplementedError();
  }

  @override
  void drawPoints(ui.PointMode pointMode, List<ui.Offset> points, ui.Paint paint) {
    throw UnimplementedError();
  }

  @override
  void drawRawPoints(ui.PointMode pointMode, Float32List points, ui.Paint paint) {
    throw UnimplementedError();
  }

  @override
  void drawVertices(ui.Vertices vertices, ui.BlendMode blendMode, ui.Paint paint) {
    throw UnimplementedError();
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
    throw UnimplementedError();
  }

  @override
  void drawRawAtlas(
    ui.Image atlas,
    Float32List rstTransforms,
    Float32List rects,
    Int32List? colors,
    ui.BlendMode? blendMode,
    ui.Rect? cullRect,
    ui.Paint paint,
  ) {
    throw UnimplementedError();
  }

  @override
  void drawShadow(
    ui.Path path,
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
