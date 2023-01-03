// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// A canvas that renders to DOM elements and CSS properties.
class SvgCanvas extends EngineCanvas with SaveElementStackTracking {
  SvgCanvas() : _id = _idCounter++ {
    rootElement.setAttribute('flt-id', _id);
    rootElement.style.overflow = 'visible';
    rootElement.append(_defs);
  }

  static int _idCounter = 1;
  final int _id;

  @override
  final SVGSVGElement rootElement = createSVGSVGElement();

  final SVGDefsElement _defs = createSVGDefsElement();

  void discard() {
    rootElement.remove();
  }

  void draw(EnginePicture picture) {
    picture.recordingCanvas!.apply(this, ui.Rect.largest);
  }

  @override
  void clear() {
    throw UnsupportedError('SvgCanvas must not be reused.');
  }

  @override
  void clipRect(ui.Rect rect, ui.ClipOp clipOp) {
    _clip(SvgClip.fromRect(rect), debugLabel: 'clipRect');
  }

  @override
  void clipRRect(ui.RRect rrect) {
    final SurfacePath path = SurfacePath();
    path.addRRect(rrect);
    _clip(SvgClip.fromPath(path), debugLabel: 'clipRRect');
  }

  @override
  void clipPath(ui.Path path) {
    _clip(SvgClip.fromPath(path), debugLabel: 'clipPath');
  }

  void _clip(SvgClip clip, { required String debugLabel }) {
    _defs.append(clip.clipPath);
    clip.clipPath.setAttribute('flt-debug', debugLabel);
    _applyCurrentTransform(clip.clipPath);
    final SVGGElement clipGroup = createSVGGElement();
    clipGroup.setAttribute('clip-path', clip.url);
    currentElement.append(clipGroup);

    // Push the element to the stack to make it the currentElement. This way the
    // clip will apply to any `draw` method until `restore` is called.
    pushElement(clipGroup);
  }

  @override
  void drawColor(ui.Color color, ui.BlendMode blendMode) {
    drawPaint(
      SurfacePaintData()
        ..color = color.value
        ..blendMode = blendMode,
    );
  }

  @override
  void drawLine(ui.Offset p1, ui.Offset p2, SurfacePaintData paint) {
    final SVGLineElement element = createSVGLineElement();
    element.setX1(p1.dx);
    element.setY1(p1.dy);
    element.setX2(p2.dx);
    element.setY2(p2.dy);
    _applyPaintToDrawing(element, ui.Rect.fromPoints(p1, p2), paint);
    _applyCurrentTransform(element);
    currentElement.append(element);
  }

  @override
  void drawPaint(SurfacePaintData paint) {
    drawRect(const ui.Rect.fromLTRB(-10000, -10000, 10000, 10000), paint);
  }

  @override
  void drawRect(ui.Rect rect, SurfacePaintData paint) {
    final SVGRectElement element = createSVGRectElement();
    element.setX(rect.left);
    element.setY(rect.top);
    element.setWidth(rect.width);
    element.setHeight(rect.height);
    _applyPaintToDrawing(element, rect, paint);
    _applyCurrentTransform(element);
    currentElement.append(element);
  }

  void _applyCurrentTransform(DomElement element) {
    setElementTransform(element, currentTransform.storage);
  }

  void _applyCurrentTransformAndOffset(DomElement element, ui.Offset offset) {
    if (!currentTransform.isIdentity()) {
      setElementTransform(
        element,
        transformWithOffset(currentTransform, offset).storage,
      );
    } else if (offset != ui.Offset.zero) {
      setElementTranslation(element, offset);
    }
  }

  @override
  void drawRRect(ui.RRect rrect, SurfacePaintData paint) {
    // TODO(yjbanov): this could be faster if specialized for rrect.
    final SurfacePath path = SurfacePath();
    path.addRRect(rrect);
    _drawPath(path, paint, debugLabel: 'drawRRect');
  }

  @override
  void drawDRRect(ui.RRect outer, ui.RRect inner, SurfacePaintData paint) {
    final SurfacePath path = SurfacePath();
    path.addDRRect(outer, inner);
    _drawPath(path, paint, debugLabel: 'drawDRRect');
  }

  @override
  void drawOval(ui.Rect rect, SurfacePaintData paint) {
    final SurfacePath path = SurfacePath();
    path.addOval(rect);
    _drawPath(path, paint, debugLabel: 'drawOval');
  }

  @override
  void drawCircle(ui.Offset c, double radius, SurfacePaintData paint) {
    final SurfacePath path = SurfacePath();
    path.addOval(ui.Rect.fromCircle(center: c, radius: radius));
    _drawPath(path, paint, debugLabel: 'drawCircle');
  }

  @override
  void drawPath(ui.Path path, SurfacePaintData paint, { String? debugLabel }) {
    _drawPath(path, paint, debugLabel: debugLabel);
  }

  SVGPathElement _drawPath(ui.Path path, SurfacePaintData paint, { String? debugLabel, ui.Offset? offset }) {
    path as SurfacePath;
    final SVGPathElement element = createSVGPathElement();
    element.setAttribute('fld-debug', debugLabel ?? 'drawPath');
    element.setAttribute('d', pathToSvg(path.pathRef));
    _applyPaintToDrawing(element, path.getBounds(), paint);
    _applyCurrentTransform(element);
    currentElement.append(element);
    return element;
  }

  @override
  void drawShadow(ui.Path path, ui.Color color, double elevation,
      bool transparentOccluder) {
    final SurfaceShadowData? shadow = computeShadow(path.getBounds(), elevation);
    if (shadow != null) {
      final SVGPathElement element = _drawPath(
        path,
        SurfacePaintData()
          ..style = ui.PaintingStyle.fill
          ..color = toShadowColor(color).value,
        offset: shadow.offset,
        debugLabel: 'drawShadow',
      );
      final ui.Rect pathBounds = path.getBounds();
      _applyBlurFilter(
        element: element,
        sigmaX: shadow.blurX / 3,
        sigmaY: shadow.blurY / 3,
        areaOfEffect: ui.Rect.fromLTRB(
          pathBounds.left - shadow.blurX,
          pathBounds.top - shadow.blurY,
          pathBounds.right + shadow.blurX,
          pathBounds.bottom + shadow.blurY,
        ),
      );
    }
  }

  @override
  void drawImage(ui.Image image, ui.Offset p, SurfacePaintData paint) {
    final SVGImageElement imageElement = _drawUnpositionedImage(image, paint);
    _applyCurrentTransformAndOffset(imageElement, p);
    currentElement.append(imageElement);
  }

  SVGImageElement _drawUnpositionedImage(ui.Image image, SurfacePaintData paint) {
    image as HtmlImage;
    final SVGImageElement imageElement = image.cloneSVGImageElement();
    _applyPaintToDrawing(
      imageElement,
      ui.Rect.fromLTWH(0, 0, image.width.toDouble(), image.height.toDouble()),
      paint,
    );
    return imageElement;
  }

  @override
  void drawImageRect(
      ui.Image image, ui.Rect src, ui.Rect dst, SurfacePaintData paint) {
    final bool needsDestinationRectangle =
        src.left != 0 ||
        src.top != 0 ||
        src.width != image.width ||
        src.height != image.height ||
        dst.width != image.width ||
        dst.height != image.height;
    if (!needsDestinationRectangle) {
      // There's no effect from the `src` or `dst` rectangles, so the image can
      // be painted using the simpler and faster route.
      drawImage(image, dst.topLeft, paint);
    } else {
      final SvgClip clip = SvgClip.fromRect(src);
      _defs.append(clip.clipPath);

      final SVGImageElement imageElement = _drawUnpositionedImage(image, paint);
      imageElement.setAttribute('clip-path', clip.url);
      imageElement.style
        ..transformOrigin = '${dst.left}px ${dst.top}px 0'
        ..transform = 'scale(calc(${dst.width / src.width}), calc(${dst.height / src.height})) '
                      'translate(${dst.left - src.left}px, ${dst.top - src.top}px)';

      if (currentTransform.isIdentity()) {
        // There's no extra transform active in the save stack, so just add the
        // element with its own transform.
        currentElement.append(imageElement);
      } else {
        // There's a non-trivial transform in the save stack. However, it cannot
        // be applied directly on the image element because the image element
        // has a non-zero transform-origin, which would apply the save stack
        // transform incorrectly. So instead, the image is wrapped in a <g>
        // element and the save stack transform is applied to it instead.
        final SVGGElement transformElement = createSVGGElement();
        _applyCurrentTransform(transformElement);
        transformElement.append(imageElement);
        currentElement.append(transformElement);
      }
    }
  }

  @override
  void drawParagraph(ui.Paragraph paragraph, ui.Offset offset) {
    paragraph as CanvasParagraph;
    assert(paragraph.isLaidOut);
    final SVGGElement paragraphElement = paragraph.toSvgElement();
    _applyCurrentTransformAndOffset(paragraphElement, offset);
    currentElement.append(paragraphElement);
  }

  @override
  void drawVertices(
      ui.Vertices vertices, ui.BlendMode blendMode, SurfacePaintData paint) {
    print('Unimplemented drawVertices');
  }

  @override
  void drawPoints(
      ui.PointMode pointMode, Float32List points, SurfacePaintData paint) {
    print('Unimplemented drawPoints');
  }

  @override
  void endOfPaint() {
    // TODO(yjbanov): maybe need to restore active saves and saveLayers here.
  }

  void _applyBlurFilter({
    required SVGElement element,
    required ui.Rect areaOfEffect,
    required double sigmaX,
    required double sigmaY,
  }) {
    final SvgFilterBuilder filterBuilder = SvgFilterBuilder(targetType: SvgFilterTargetType.svg)
      ..setFeGaussianBlur(
        sigmaX: sigmaX,
        sigmaY: sigmaY,
        areaOfEffect: areaOfEffect,
      );
    final SvgFilter filter = filterBuilder.build(_defs);
    filter.applyToSvg(element);
  }

  void _applyPaintToDrawing(SVGElement element, ui.Rect drawableBounds, SurfacePaintData paint) {
    final bool isStroke = paint.style == ui.PaintingStyle.stroke;
    final double strokeWidth = paint.strokeWidth ?? 0.0;
    final DomCSSStyleDeclaration style = element.style;
    final String cssColor = paint.color == null ? '#000000' : colorValueToCssString(paint.color)!;

    final ui.MaskFilter? maskFilter = paint.maskFilter;
    if (maskFilter != null) {
      _applyBlurFilter(
        element: element,
        sigmaX: maskFilter.webOnlySigma,
        sigmaY: maskFilter.webOnlySigma,
        areaOfEffect: drawableBounds.inflate(maskFilter.webOnlySigma * 5),
      );
    }

    if (isStroke) {
      element.setAttribute('stroke', cssColor);
      element.setAttribute('fill', 'none');
      if (strokeWidth == 0) {
        // In Flutter 0 means "hairline", i.e. exactly 1 pixel wide line, which in
        // SVG is expressed as a non-scaling vector effect.
        style.vectorEffect = 'non-scaling-stroke';
      } else {
        element.setAttribute('stroke-width', '${strokeWidth.toStringAsFixed(3)}px');
      }
    } else {
      element.setAttribute('fill', cssColor);
      element.setAttribute('stroke', 'none');
    }
  }
}
