import 'dart:ffi';
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import 'paint.dart';
import 'path.dart';
import 'picture.dart';
import 'raw/raw_canvas.dart';
import 'raw/raw_memory.dart';
import 'raw/raw_picture.dart';

class SkwasmCanvas implements ui.Canvas {
  factory SkwasmCanvas(SkwasmPictureRecorder recorder, [ui.Rect cullRect = ui.Rect.largest]) =>
    SkwasmCanvas.fromHandle(withStackScope((StackScope s) =>
      pictureRecorderBeginRecording(recorder.handle, s.convertRect(cullRect))
    ));

  SkwasmCanvas.fromHandle(this._handle);
  CanvasHandle _handle;

  void delete() {
    canvasDestroy(_handle);
  }

  @override
  void save() {
    canvasSave(_handle);
  }

  @override
  void saveLayer(ui.Rect? bounds, ui.Paint uiPaint) {
    assert(uiPaint is SkwasmPaint);
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    if (bounds != null) {
      withStackScope((StackScope s) {
        canvasSaveLayer(_handle, s.convertRect(bounds), paint.handle);
      });
    } else {
      canvasSaveLayer(_handle, nullptr, paint.handle);
    }
  }

  @override
  void restore() {
    canvasRestore(_handle);
  }

  @override
  int getSaveCount() => canvasGetSaveCount(_handle);

  @override
  void translate(double dx, double dy) => canvasTranslate(_handle, dx, dy);

  @override
  void scale(double sx, [double? sy]) => canvasScale(_handle, sx, sy ?? sx);

  @override
  void rotate(double radians) => canvasRotate(_handle, ui.toDegrees(radians));

  @override
  void skew(double sx, double sy) => canvasSkew(_handle, sx, sy);

  @override
  void transform(Float64List matrix4) {
    withStackScope((StackScope s) {
      canvasTransform(_handle, s.convertMatrix4toSkM44(matrix4));
    });
  }

  @override
  void clipRect(ui.Rect rect,
      {ui.ClipOp clipOp = ui.ClipOp.intersect, bool doAntiAlias = true}) {
    withStackScope((StackScope s) {
      canvasClipRect(_handle, s.convertRect(rect), clipOp.index, doAntiAlias);
    });
  }

  @override
  void clipRRect(ui.RRect rrect, {bool doAntiAlias = true}) {
    withStackScope((StackScope s) {
      canvasClipRRect(_handle, s.convertRRect(rrect), doAntiAlias);
    });
  }

  @override
  void clipPath(ui.Path uiPath, {bool doAntiAlias = true}) {
    assert(uiPath is SkwasmPath);
    final SkwasmPath path = uiPath as SkwasmPath;
    canvasClipPath(_handle, path.handle, doAntiAlias);
  }

  @override
  void drawColor(ui.Color color, ui.BlendMode blendMode) => 
    canvasDrawColor(_handle, color.value, blendMode.index);

  @override
  void drawLine(ui.Offset p1, ui.Offset p2, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    canvasDrawLine(_handle, p1.dx, p1.dy, p2.dx, p2.dy, paint.handle);
  }

  @override
  void drawPaint(ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    canvasDrawPaint(_handle, paint.handle);
  }

  @override
  void drawRect(ui.Rect rect, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvasDrawRect(_handle, s.convertRect(rect), paint.handle);
    });
  }

  @override
  void drawRRect(ui.RRect rrect, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvasDrawRRect(_handle, s.convertRRect(rrect), paint.handle);
    });
  }

  @override
  void drawDRRect(ui.RRect outer, ui.RRect inner, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvasDrawDRRect(
          _handle, s.convertRRect(outer), s.convertRRect(inner), paint.handle);
    });
  }

  @override
  void drawOval(ui.Rect rect, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvasDrawOval(_handle, s.convertRect(rect), paint.handle);
    });
  }

  @override
  void drawCircle(ui.Offset center, double radius, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    canvasDrawCircle(_handle, center.dx, center.dy, radius, paint.handle);
  }

  @override
  void drawArc(ui.Rect rect, double startAngle, double sweepAngle, bool useCenter,
      ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    withStackScope((StackScope s) {
      canvasDrawArc(
          _handle,
          s.convertRect(rect),
          ui.toDegrees(startAngle),
          ui.toDegrees(sweepAngle),
          useCenter,
          paint.handle);
    });
  }

  @override
  void drawPath(ui.Path uiPath, ui.Paint uiPaint) {
    final SkwasmPaint paint = uiPaint as SkwasmPaint;
    final SkwasmPath path = uiPath as SkwasmPath;
    canvasDrawPath(_handle, path.handle, paint.handle);
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
    canvasDrawPicture(_handle, (picture as SkwasmPicture).handle);
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
