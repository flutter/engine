// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// A Dart wrapper around Skia's SKCanvas.
class SkCanvas {
  final js.JsObject skCanvas;

  SkCanvas(this.skCanvas);

  int get saveCount => skCanvas.callMethod('getSaveCount');

  void clipPath(ui.Path path, bool doAntiAlias) {
    final SkPath skPath = path;
    final js.JsObject intersectClipOp = canvasKit['ClipOp']['Intersect'];
    skCanvas.callMethod('clipPath', <dynamic>[
      skPath._skPath,
      intersectClipOp,
      doAntiAlias,
    ]);
  }

  void clipRRect(ui.RRect rrect, bool doAntiAlias) {
    final js.JsObject intersectClipOp = canvasKit['ClipOp']['Intersect'];
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
        skClipOp = canvasKit['ClipOp']['Difference'];
        break;
      case ui.ClipOp.intersect:
        skClipOp = canvasKit['ClipOp']['Intersect'];
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
    ui.Paint paint,
  ) {
    skCanvas.callMethod('drawArc', <dynamic>[
      makeSkRect(oval),
      startAngle,
      sweepAngle,
      useCenter,
      makeSkPaint(paint),
    ]);
  }

  void drawCircle(ui.Offset c, double radius, ui.Paint paint) {
    skCanvas.callMethod('drawCircle', <dynamic>[
      c.dx,
      c.dy,
      radius,
      makeSkPaint(paint),
    ]);
  }

  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    skCanvas.callMethod('drawColor', <dynamic>[
      color.value,
      makeSkBlendMode(blendMode),
    ]);
  }

  void drawDRRect(ui.RRect outer, ui.RRect inner, ui.Paint paint) {
    skCanvas.callMethod('drawDRRect', <js.JsObject>[
      makeSkRRect(outer),
      makeSkRRect(inner),
      makeSkPaint(paint),
    ]);
  }

  void drawImage(ui.Image image, ui.Offset offset, ui.Paint paint) {
    final SkImage skImage = image;
    skCanvas.callMethod('drawImage', <dynamic>[
      skImage.skImage,
      offset.dx,
      offset.dy,
      makeSkPaint(paint),
    ]);
  }

  void drawImageRect(ui.Image image, ui.Rect src, ui.Rect dst, ui.Paint paint) {
    final SkImage skImage = image;
    skCanvas.callMethod('drawImageRect', <dynamic>[
      skImage.skImage,
      makeSkRect(src),
      makeSkRect(dst),
      makeSkPaint(paint),
      false,
    ]);
  }

  void drawLine(ui.Offset p1, ui.Offset p2, ui.Paint paint) {
    skCanvas.callMethod('drawLine', <dynamic>[
      p1.dx,
      p1.dy,
      p2.dx,
      p2.dy,
      makeSkPaint(paint),
    ]);
  }

  void drawOval(ui.Rect rect, ui.Paint paint) {
    skCanvas.callMethod('drawOval', <js.JsObject>[
      makeSkRect(rect),
      makeSkPaint(paint),
    ]);
  }

  void drawPaint(ui.Paint paint) {
    skCanvas.callMethod('drawPaint', <js.JsObject>[makeSkPaint(paint)]);
  }

  void drawParagraph(ui.Paragraph paragraph, ui.Offset offset) {
    // TODO(het): This doesn't support most paragraph features. We are just
    // creating a font from the family and size, and drawing it with
    // ShapedText.
    final EngineParagraph engineParagraph = paragraph;
    final ParagraphGeometricStyle style = engineParagraph.geometricStyle;
    final js.JsObject skFont = skiaFontCollection.getFont(
        style.effectiveFontFamily, style.fontSize ?? 12.0);
    final js.JsObject skShapedTextOpts = js.JsObject.jsify(<String, dynamic>{
      'font': skFont,
      'leftToRight': true,
      'text': engineParagraph.plainText,
      'width': engineParagraph.width + 1,
    });
    final js.JsObject skShapedText =
        js.JsObject(canvasKit['ShapedText'], <js.JsObject>[skShapedTextOpts]);
    skCanvas.callMethod('drawText', <dynamic>[
      skShapedText,
      offset.dx + engineParagraph._alignOffset,
      offset.dy,
      makeSkPaint(engineParagraph._paint)
    ]);
  }

  void drawPath(ui.Path path, ui.Paint paint) {
    final js.JsObject skPaint = makeSkPaint(paint);
    final SkPath enginePath = path;
    final js.JsObject skPath = enginePath._skPath;
    skCanvas.callMethod('drawPath', <js.JsObject>[skPath, skPaint]);
  }

  void drawRRect(ui.RRect rrect, ui.Paint paint) {
    skCanvas.callMethod('drawRRect', <js.JsObject>[
      makeSkRRect(rrect),
      makeSkPaint(paint),
    ]);
  }

  void drawRect(ui.Rect rect, ui.Paint paint) {
    final js.JsObject skRect = makeSkRect(rect);
    final js.JsObject skPaint = makeSkPaint(paint);
    skCanvas.callMethod('drawRect', <js.JsObject>[skRect, skPaint]);
  }

  void drawShadow(ui.Path path, ui.Color color, double elevation,
      bool transparentOccluder) {
    drawSkShadow(skCanvas, path, color, elevation, transparentOccluder,
        ui.window.devicePixelRatio);
  }

  void drawVertices(
      ui.Vertices vertices, ui.BlendMode blendMode, ui.Paint paint) {
    SkVertices skVertices = vertices;
    skCanvas.callMethod('drawVertices', <js.JsObject>[
      skVertices.skVertices,
      makeSkBlendMode(blendMode),
      makeSkPaint(paint)
    ]);
  }

  void restore() {
    skCanvas.callMethod('restore');
  }

  void rotate(double radians) {
    skCanvas
        .callMethod('rotate', <double>[radians * 180.0 / math.pi, 0.0, 0.0]);
  }

  void save() {
    skCanvas.callMethod('save');
  }

  void saveLayer(ui.Rect bounds, ui.Paint paint) {
    skCanvas.callMethod('saveLayer', <js.JsObject>[
      makeSkRect(bounds),
      makeSkPaint(paint),
    ]);
  }

  void saveLayerWithoutBounds(ui.Paint paint) {
    skCanvas.callMethod('saveLayer', <js.JsObject>[null, makeSkPaint(paint)]);
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
}
