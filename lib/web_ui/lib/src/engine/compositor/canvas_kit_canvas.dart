// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// An implementation of [ui.Canvas] that is backed by a CanvasKit canvas.
class CanvasKitCanvas implements ui.Canvas {
  final SkLayerCanvas _canvas;

  factory CanvasKitCanvas(ui.PictureRecorder recorder, [ui.Rect cullRect]) {
    assert(recorder != null);
    if (recorder.isRecording)
      throw ArgumentError(
          '"recorder" must not already be associated with another Canvas.');
    cullRect ??= ui.Rect.largest;
    final SkPictureRecorder skRecorder = recorder;
    return CanvasKitCanvas._(skRecorder.beginRecording(cullRect));
  }

  CanvasKitCanvas._(this._canvas);

  void save() {
    _canvas.save();
  }

  void saveLayer(ui.Rect bounds, ui.Paint paint) {
    assert(paint != null);
    if (bounds == null) {
      _saveLayerWithoutBounds(paint);
    } else {
      assert(rectIsValid(bounds));
      _saveLayer(bounds, paint);
    }
  }

  void _saveLayerWithoutBounds(ui.Paint paint) {
  }

  void _saveLayer(double left, double top, double right, double bottom,
      List<dynamic> paintObjects, ByteData paintData) native 'Canvas_saveLayer';

  void restore() native 'Canvas_restore';

  int getSaveCount() native 'Canvas_getSaveCount';

  void translate(double dx, double dy) native 'Canvas_translate';

  void scale(double sx, [double sy]) => _scale(sx, sy ?? sx);

  void _scale(double sx, double sy) native 'Canvas_scale';

  void rotate(double radians) native 'Canvas_rotate';

  void skew(double sx, double sy) native 'Canvas_skew';

  void transform(Float64List matrix4) {
    assert(matrix4 != null);
    if (matrix4.length != 16)
      throw ArgumentError('"matrix4" must have 16 entries.');
    _transform(matrix4);
  }

  void _transform(Float64List matrix4) native 'Canvas_transform';

  void clipRect(ui.Rect rect,
      {ClipOp clipOp = ClipOp.intersect, bool doAntiAlias = true}) {
    assert(_rectIsValid(rect));
    assert(clipOp != null);
    assert(doAntiAlias != null);
    _clipRect(rect.left, rect.top, rect.right, rect.bottom, clipOp.index,
        doAntiAlias);
  }

  void _clipRect(double left, double top, double right, double bottom,
      int clipOp, bool doAntiAlias) native 'Canvas_clipRect';

  void clipRRect(ui.RRect rrect, {bool doAntiAlias = true}) {
    assert(_rrectIsValid(rrect));
    assert(doAntiAlias != null);
    _clipRRect(rrect._value32, doAntiAlias);
  }

  void _clipRRect(Float32List rrect, bool doAntiAlias)
      native 'Canvas_clipRRect';

  void clipPath(ui.Path path, {bool doAntiAlias = true}) {
    assert(path != null); // path is checked on the engine side
    assert(doAntiAlias != null);
    _clipPath(path, doAntiAlias);
  }

  void _clipPath(ui.Path path, bool doAntiAlias) native 'Canvas_clipPath';

  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    assert(color != null);
    assert(blendMode != null);
    _drawColor(color.value, blendMode.index);
  }

  void _drawColor(int color, int blendMode) native 'Canvas_drawColor';

  void drawLine(Offset p1, Offset p2, Paint paint) {
    assert(_offsetIsValid(p1));
    assert(_offsetIsValid(p2));
    assert(paint != null);
    _drawLine(p1.dx, p1.dy, p2.dx, p2.dy, paint._objects, paint._data);
  }

  void _drawLine(double x1, double y1, double x2, double y2,
      List<dynamic> paintObjects, ByteData paintData) native 'Canvas_drawLine';

  void drawPaint(Paint paint) {
    assert(paint != null);
    _drawPaint(paint._objects, paint._data);
  }

  void _drawPaint(List<dynamic> paintObjects, ByteData paintData)
      native 'Canvas_drawPaint';

  void drawRect(Rect rect, Paint paint) {
    assert(_rectIsValid(rect));
    assert(paint != null);
    _drawRect(rect.left, rect.top, rect.right, rect.bottom, paint._objects,
        paint._data);
  }

  void _drawRect(double left, double top, double right, double bottom,
      List<dynamic> paintObjects, ByteData paintData) native 'Canvas_drawRect';

  void drawRRect(RRect rrect, Paint paint) {
    assert(_rrectIsValid(rrect));
    assert(paint != null);
    _drawRRect(rrect._value32, paint._objects, paint._data);
  }

  void _drawRRect(Float32List rrect, List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawRRect';

  void drawDRRect(RRect outer, RRect inner, Paint paint) {
    assert(_rrectIsValid(outer));
    assert(_rrectIsValid(inner));
    assert(paint != null);
    _drawDRRect(outer._value32, inner._value32, paint._objects, paint._data);
  }

  void _drawDRRect(
      Float32List outer,
      Float32List inner,
      List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawDRRect';

  void drawOval(Rect rect, Paint paint) {
    assert(_rectIsValid(rect));
    assert(paint != null);
    _drawOval(rect.left, rect.top, rect.right, rect.bottom, paint._objects,
        paint._data);
  }

  void _drawOval(double left, double top, double right, double bottom,
      List<dynamic> paintObjects, ByteData paintData) native 'Canvas_drawOval';

  void drawCircle(Offset c, double radius, Paint paint) {
    assert(_offsetIsValid(c));
    assert(paint != null);
    _drawCircle(c.dx, c.dy, radius, paint._objects, paint._data);
  }

  void _drawCircle(
      double x,
      double y,
      double radius,
      List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawCircle';

  void drawArc(Rect rect, double startAngle, double sweepAngle, bool useCenter,
      Paint paint) {
    assert(_rectIsValid(rect));
    assert(paint != null);
    _drawArc(rect.left, rect.top, rect.right, rect.bottom, startAngle,
        sweepAngle, useCenter, paint._objects, paint._data);
  }

  void _drawArc(
      double left,
      double top,
      double right,
      double bottom,
      double startAngle,
      double sweepAngle,
      bool useCenter,
      List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawArc';

  void drawPath(Path path, Paint paint) {
    assert(path != null); // path is checked on the engine side
    assert(paint != null);
    _drawPath(path, paint._objects, paint._data);
  }

  void _drawPath(Path path, List<dynamic> paintObjects, ByteData paintData)
      native 'Canvas_drawPath';

  void drawImage(ui.Image image, ui.Offset p, ui.Paint paint) {
    assert(image != null); // image is checked on the engine side
    assert(_offsetIsValid(p));
    assert(paint != null);
    _drawImage(image, p.dx, p.dy, paint._objects, paint._data);
  }

  void _drawImage(Image image, double x, double y, List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawImage';

  void drawImageRect(ui.Image image, ui.Rect src, ui.Rect dst, ui.Paint paint) {
    assert(image != null); // image is checked on the engine side
    assert(_rectIsValid(src));
    assert(_rectIsValid(dst));
    assert(paint != null);
    _drawImageRect(image, src.left, src.top, src.right, src.bottom, dst.left,
        dst.top, dst.right, dst.bottom, paint._objects, paint._data);
  }

  void _drawImageRect(
      ui.Image image,
      double srcLeft,
      double srcTop,
      double srcRight,
      double srcBottom,
      double dstLeft,
      double dstTop,
      double dstRight,
      double dstBottom,
      List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawImageRect';

  void drawImageNine(
      ui.Image image, ui.Rect center, ui.Rect dst, ui.Paint paint) {
    assert(image != null); // image is checked on the engine side
    assert(_rectIsValid(center));
    assert(_rectIsValid(dst));
    assert(paint != null);
    _drawImageNine(image, center.left, center.top, center.right, center.bottom,
        dst.left, dst.top, dst.right, dst.bottom, paint._objects, paint._data);
  }

  void _drawImageNine(
      Image image,
      double centerLeft,
      double centerTop,
      double centerRight,
      double centerBottom,
      double dstLeft,
      double dstTop,
      double dstRight,
      double dstBottom,
      List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawImageNine';

  void drawPicture(Picture picture) {
    assert(picture != null); // picture is checked on the engine side
    _drawPicture(picture);
  }

  void _drawPicture(Picture picture) native 'Canvas_drawPicture';

  void drawParagraph(Paragraph paragraph, Offset offset) {
    assert(paragraph != null);
    assert(_offsetIsValid(offset));
    paragraph._paint(this, offset.dx, offset.dy);
  }

  void drawPoints(
      ui.PointMode pointMode, List<ui.Offset> points, ui.Paint paint) {
    assert(pointMode != null);
    assert(points != null);
    assert(paint != null);
    _drawPoints(
        paint._objects, paint._data, pointMode.index, _encodePointList(points));
  }

  void drawRawPoints(
      ui.PointMode pointMode, Float32List points, ui.Paint paint) {
    assert(pointMode != null);
    assert(points != null);
    assert(paint != null);
    if (points.length % 2 != 0)
      throw ArgumentError('"points" must have an even number of values.');
    _drawPoints(paint._objects, paint._data, pointMode.index, points);
  }

  void _drawPoints(List<dynamic> paintObjects, ByteData paintData,
      int pointMode, Float32List points) native 'Canvas_drawPoints';

  void drawVertices(Vertices vertices, BlendMode blendMode, Paint paint) {
    assert(vertices != null); // vertices is checked on the engine side
    assert(paint != null);
    assert(blendMode != null);
    _drawVertices(vertices, blendMode.index, paint._objects, paint._data);
  }

  void _drawVertices(
      Vertices vertices,
      int blendMode,
      List<dynamic> paintObjects,
      ByteData paintData) native 'Canvas_drawVertices';

  void drawAtlas(Image atlas, List<RSTransform> transforms, List<Rect> rects,
      List<Color> colors, BlendMode blendMode, Rect cullRect, Paint paint) {
    assert(atlas != null); // atlas is checked on the engine side
    assert(transforms != null);
    assert(rects != null);
    assert(colors != null);
    assert(blendMode != null);
    assert(paint != null);

    final int rectCount = rects.length;
    if (transforms.length != rectCount)
      throw ArgumentError('"transforms" and "rects" lengths must match.');
    if (colors.isNotEmpty && colors.length != rectCount)
      throw ArgumentError(
          'If non-null, "colors" length must match that of "transforms" and "rects".');

    final Float32List rstTransformBuffer = Float32List(rectCount * 4);
    final Float32List rectBuffer = Float32List(rectCount * 4);

    for (int i = 0; i < rectCount; ++i) {
      final int index0 = i * 4;
      final int index1 = index0 + 1;
      final int index2 = index0 + 2;
      final int index3 = index0 + 3;
      final RSTransform rstTransform = transforms[i];
      final Rect rect = rects[i];
      assert(_rectIsValid(rect));
      rstTransformBuffer[index0] = rstTransform.scos;
      rstTransformBuffer[index1] = rstTransform.ssin;
      rstTransformBuffer[index2] = rstTransform.tx;
      rstTransformBuffer[index3] = rstTransform.ty;
      rectBuffer[index0] = rect.left;
      rectBuffer[index1] = rect.top;
      rectBuffer[index2] = rect.right;
      rectBuffer[index3] = rect.bottom;
    }

    final Int32List colorBuffer =
        colors.isEmpty ? null : _encodeColorList(colors);
    final Float32List cullRectBuffer = cullRect?._value32;

    _drawAtlas(paint._objects, paint._data, atlas, rstTransformBuffer,
        rectBuffer, colorBuffer, blendMode.index, cullRectBuffer);
  }

  void drawRawAtlas(Image atlas, Float32List rstTransforms, Float32List rects,
      Int32List colors, BlendMode blendMode, Rect cullRect, Paint paint) {
    assert(atlas != null); // atlas is checked on the engine side
    assert(rstTransforms != null);
    assert(rects != null);
    assert(colors != null);
    assert(blendMode != null);
    assert(paint != null);

    final int rectCount = rects.length;
    if (rstTransforms.length != rectCount)
      throw ArgumentError('"rstTransforms" and "rects" lengths must match.');
    if (rectCount % 4 != 0)
      throw ArgumentError(
          '"rstTransforms" and "rects" lengths must be a multiple of four.');
    if (colors != null && colors.length * 4 != rectCount)
      throw ArgumentError(
          'If non-null, "colors" length must be one fourth the length of "rstTransforms" and "rects".');

    _drawAtlas(paint._objects, paint._data, atlas, rstTransforms, rects, colors,
        blendMode.index, cullRect?._value32);
  }

  void _drawAtlas(
      List<dynamic> paintObjects,
      ByteData paintData,
      Image atlas,
      Float32List rstTransforms,
      Float32List rects,
      Int32List colors,
      int blendMode,
      Float32List cullRect) native 'Canvas_drawAtlas';

  void drawShadow(
      Path path, Color color, double elevation, bool transparentOccluder) {
    assert(path != null); // path is checked on the engine side
    assert(color != null);
    assert(transparentOccluder != null);
    _drawShadow(path, color.value, elevation, transparentOccluder);
  }

  void _drawShadow(Path path, int color, double elevation,
      bool transparentOccluder) native 'Canvas_drawShadow';
}
