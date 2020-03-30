// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

/// A Dart wrapper around Skia's SKCanvas.
class SkCanvas {
  final js.JsObject skCanvas;

  SkCanvas(this.skCanvas);

  int get saveCount => unsafeCast<int>(skCanvas.callMethod('getSaveCount'));

  void clear(ui.Color color) {
    skCanvas.callMethod('clear', <int>[color.value]);
  }

  void clipPath(SkPath path, bool doAntiAlias) {
    final SkPath skPath = path;
    final js.JsObject intersectClipOp = unsafeCast<js.JsObject>(canvasKit['ClipOp']['Intersect']);
    skCanvas.callMethod('clipPath', <dynamic>[
      skPath._skPath,
      intersectClipOp,
      doAntiAlias,
    ]);
  }

  void clipRRect(ui.RRect rrect, bool doAntiAlias) {
    final js.JsObject intersectClipOp = unsafeCast<js.JsObject>(canvasKit['ClipOp']['Intersect']);
    skCanvas.callMethod('clipRRect', <dynamic>[
      makeSkRRect(rrect),
      intersectClipOp,
      doAntiAlias,
    ]);
  }

  void clipRect(ui.Rect rect, ui.ClipOp clipOp, bool doAntiAlias) {
    js.JsObject skClipOp;
    switch (clipOp) {
      case ui.ClipOp.difference:
        skClipOp = unsafeCast<js.JsObject>(canvasKit['ClipOp']['Difference']);
        break;
      case ui.ClipOp.intersect:
        skClipOp = unsafeCast<js.JsObject>(canvasKit['ClipOp']['Intersect']);
        break;
    }

    skCanvas.callMethod(
        'clipRect', <dynamic>[makeSkRect(rect), skClipOp, doAntiAlias]);
  }

  void drawArc(
    ui.Rect oval,
    double startAngle,
    double sweepAngle,
    bool useCenter,
    SkPaint paint,
  ) {
    const double toDegrees = 180 / math.pi;
    skCanvas.callMethod('drawArc', <dynamic>[
      makeSkRect(oval),
      startAngle * toDegrees,
      sweepAngle * toDegrees,
      useCenter,
      paint.skiaObject,
    ]);
  }

  void drawAtlasRaw(
    SkPaint paint,
    SkImage atlas,
    Float32List rstTransforms,
    Float32List rects,
    Int32List colors,
    ui.BlendMode blendMode,
  ) {
    skCanvas.callMethod('drawAtlas', <dynamic>[
      atlas.skImage,
      rects,
      rstTransforms,
      paint.skiaObject,
      makeSkBlendMode(blendMode),
      colors,
    ]);
  }

  void drawCircle(ui.Offset c, double radius, SkPaint paint) {
    skCanvas.callMethod('drawCircle', <dynamic>[
      c.dx,
      c.dy,
      radius,
      paint.skiaObject,
    ]);
  }

  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    skCanvas.callMethod('drawColor', <dynamic>[
      color.value,
      makeSkBlendMode(blendMode),
    ]);
  }

  void drawDRRect(ui.RRect outer, ui.RRect inner, SkPaint paint) {
    skCanvas.callMethod('drawDRRect', <js.JsObject>[
      makeSkRRect(outer),
      makeSkRRect(inner),
      paint.skiaObject,
    ]);
  }

  void drawImage(SkImage image, ui.Offset offset, SkPaint paint) {
    skCanvas.callMethod('drawImage', <dynamic>[
      image.skImage,
      offset.dx,
      offset.dy,
      paint.skiaObject,
    ]);
  }

  void drawImageRect(SkImage image, ui.Rect src, ui.Rect dst, SkPaint paint) {
    skCanvas.callMethod('drawImageRect', <dynamic>[
      image.skImage,
      makeSkRect(src),
      makeSkRect(dst),
      paint.skiaObject,
      false,
    ]);
  }

  void drawImageNine(
      SkImage image, ui.Rect center, ui.Rect dst, SkPaint paint) {
    skCanvas.callMethod('drawImageNine', <dynamic>[
      image.skImage,
      makeSkRect(center),
      makeSkRect(dst),
      paint.skiaObject,
    ]);
  }

  void drawLine(ui.Offset p1, ui.Offset p2, SkPaint paint) {
    skCanvas.callMethod('drawLine', <dynamic>[
      p1.dx,
      p1.dy,
      p2.dx,
      p2.dy,
      paint.skiaObject,
    ]);
  }

  void drawOval(ui.Rect rect, SkPaint paint) {
    skCanvas.callMethod('drawOval', <js.JsObject>[
      makeSkRect(rect),
      paint.skiaObject,
    ]);
  }

  void drawPaint(SkPaint paint) {
    skCanvas.callMethod('drawPaint', <js.JsObject>[paint.skiaObject]);
  }

  void drawParagraph(SkParagraph paragraph, ui.Offset offset) {
    skCanvas.callMethod('drawParagraph', <dynamic>[
      paragraph.skParagraph,
      offset.dx,
      offset.dy,
    ]);
  }

  void drawPath(SkPath path, SkPaint paint) {
    final js.JsObject skPaint = paint.skiaObject;
    final js.JsObject skPath = path._skPath;
    skCanvas.callMethod('drawPath', <js.JsObject>[skPath, skPaint]);
  }

  void drawPicture(SkPicture picture) {
    skCanvas.callMethod('drawPicture', <js.JsObject>[picture.skPicture]);
  }

  void drawPoints(SkPaint paint, ui.PointMode pointMode, Float32List points) {
    skCanvas.callMethod('drawPoints', <dynamic>[
      makeSkPointMode(pointMode),
      points,
      paint.skiaObject,
    ]);
  }

  void drawRRect(ui.RRect rrect, SkPaint paint) {
    skCanvas.callMethod('drawRRect', <js.JsObject>[
      makeSkRRect(rrect),
      paint.skiaObject,
    ]);
  }

  void drawRect(ui.Rect rect, SkPaint paint) {
    final js.JsObject skRect = makeSkRect(rect);
    final js.JsObject skPaint = paint.skiaObject;
    skCanvas.callMethod('drawRect', <js.JsObject>[skRect, skPaint]);
  }

  void drawShadow(SkPath path, ui.Color color, double elevation,
      bool transparentOccluder) {
    drawSkShadow(skCanvas, path, color, elevation, transparentOccluder,
        ui.window.devicePixelRatio);
  }

  void drawVertices(
      SkVertices vertices, ui.BlendMode blendMode, SkPaint paint) {
    skCanvas.callMethod('drawVertices', <js.JsObject>[
      vertices.skVertices,
      makeSkBlendMode(blendMode),
      paint.skiaObject
    ]);
  }

  void restore() {
    skCanvas.callMethod('restore');
  }

  void restoreToCount(int count) {
    skCanvas.callMethod('restoreToCount', <int>[count]);
  }

  void rotate(double radians) {
    skCanvas
        .callMethod('rotate', <double>[radians * 180.0 / math.pi, 0.0, 0.0]);
  }

  int save() {
    return unsafeCast<int>(skCanvas.callMethod('save'));
  }

  void saveLayer(ui.Rect bounds, SkPaint paint) {
    skCanvas.callMethod('saveLayer', <js.JsObject>[
      makeSkRect(bounds),
      paint.skiaObject,
    ]);
  }

  void saveLayerWithoutBounds(SkPaint paint) {
    skCanvas.callMethod('saveLayer', <js.JsObject>[null, paint.skiaObject]);
  }

  void saveLayerWithFilter(ui.Rect bounds, SkImageFilter filter) {
    return skCanvas.callMethod(
      'saveLayer',
      <dynamic>[
        null,
        filter.skImageFilter,
        0,
        makeSkRect(bounds),
      ],
    );
  }

  void scale(double sx, double sy) {
    skCanvas.callMethod('scale', <double>[sx, sy]);
  }

  void skew(double sx, double sy) {
    skCanvas.callMethod('skew', <double>[sx, sy]);
  }

  void transform(Float64List matrix4) {
    skCanvas.callMethod('concat', <js.JsArray<double>>[makeSkMatrix(matrix4)]);
  }

  void translate(double dx, double dy) {
    skCanvas.callMethod('translate', <double>[dx, dy]);
  }

  void flush() {
    skCanvas.callMethod('flush');
  }
}
