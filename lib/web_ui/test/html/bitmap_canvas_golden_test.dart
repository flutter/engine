// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:math' as math;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';

import 'package:web_engine_tester/golden_tester.dart';
import 'screenshot.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  final Rect region = Rect.fromLTWH(0, 0, 500, 100);

  late BitmapCanvas canvas;

  void appendToScene() {
    // Create a <flt-scene> element to make sure our CSS reset applies correctly.
    final html.Element testScene = html.Element.tag('flt-scene');
    testScene.append(canvas.rootElement);
    domRenderer.glassPaneShadow!.querySelector('flt-scene-host')!.append(testScene);
  }

  setUpStableTestFonts();

  tearDown(() {
    domRenderer.glassPaneShadow?.querySelector('flt-scene')?.remove();
  });

  /// Draws several lines, some aligned precisely with the pixel grid, and some
  /// that are offset by 0.5 vertically or horizontally.
  ///
  /// The produced picture stresses the antialiasing generated by the browser
  /// when positioning and rasterizing `<canvas>` tags. Aliasing artifacts can
  /// be seen depending on pixel alignment and whether antialiasing happens
  /// before or after rasterization.
  void drawMisalignedLines(BitmapCanvas canvas) {
    final SurfacePaintData linePaint = (SurfacePaint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 1)
        .paintData;

    final SurfacePaintData fillPaint =
        (SurfacePaint()..style = PaintingStyle.fill).paintData;

    canvas.translate(10, 10);

    canvas.drawRect(
      const Rect.fromLTWH(0, 0, 40, 40),
      linePaint,
    );

    canvas.drawLine(
      const Offset(10, 0),
      const Offset(10, 40),
      linePaint,
    );

    canvas.drawLine(
      const Offset(20.5, 0),
      const Offset(20, 40),
      linePaint,
    );

    canvas.drawCircle(const Offset(30, 10), 3, fillPaint);
    canvas.drawCircle(const Offset(30.5, 30), 3, fillPaint);
  }

  test('renders pixels that are not aligned inside the canvas', () async {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 60, 60),
        RenderStrategy());

    drawMisalignedLines(canvas);

    appendToScene();

    await matchGoldenFile('misaligned_pixels_in_canvas_test.png', region: region);
  });

  test('compensates for misalignment of the canvas', () async {
    // Notice the 0.5 offset in the bounds rectangle. It's what causes the
    // misalignment of canvas relative to the pixel grid. BitmapCanvas will
    // shift its position back to 0.0 and at the same time it will it will
    // compensate by shifting the contents of the canvas in the opposite
    // direction.
    canvas = BitmapCanvas(const Rect.fromLTWH(0.5, 0.5, 60, 60),
        RenderStrategy());
    canvas.clipRect(const Rect.fromLTWH(0, 0, 50, 50), ClipOp.intersect);
    drawMisalignedLines(canvas);

    appendToScene();

    await matchGoldenFile('misaligned_canvas_test.png', region: region,
      maxDiffRatePercent: 1.0);
  });

  test('fill the whole canvas with color even when transformed', () async {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 50, 50),
        RenderStrategy());
    canvas.clipRect(const Rect.fromLTWH(0, 0, 50, 50), ClipOp.intersect);
    canvas.translate(25, 25);
    canvas.drawColor(const Color.fromRGBO(0, 255, 0, 1.0), BlendMode.src);

    appendToScene();

    await matchGoldenFile('bitmap_canvas_fills_color_when_transformed.png',
        region: region,
        maxDiffRatePercent: 5.0);
  });

  test('fill the whole canvas with paint even when transformed', () async {
    canvas = BitmapCanvas(const Rect.fromLTWH(0, 0, 50, 50),
        RenderStrategy());
    canvas.clipRect(const Rect.fromLTWH(0, 0, 50, 50), ClipOp.intersect);
    canvas.translate(25, 25);
    canvas.drawPaint(SurfacePaintData()
      ..color = const Color.fromRGBO(0, 255, 0, 1.0)
      ..style = PaintingStyle.fill);

    appendToScene();

    await matchGoldenFile('bitmap_canvas_fills_paint_when_transformed.png',
        region: region,
        maxDiffRatePercent: 5.0);
  });

  // This test reproduces text blurriness when two pieces of text appear inside
  // two nested clips:
  //
  //   ┌───────────────────────┐
  //   │   text in outer clip  │
  //   │ ┌────────────────────┐│
  //   │ │ text in inner clip ││
  //   │ └────────────────────┘│
  //   └───────────────────────┘
  //
  // This test clips using canvas. See a similar test in `compositing_golden_test.dart`,
  // which clips using layers.
  //
  // More details: https://github.com/flutter/flutter/issues/32274
  test('renders clipped DOM text with high quality', () async {
    final EngineParagraph paragraph =
        (ParagraphBuilder(ParagraphStyle(fontFamily: 'Roboto'))
          ..addText('Am I blurry?')).build() as EngineParagraph;
    paragraph.layout(const ParagraphConstraints(width: 1000));

    final Rect canvasSize = Rect.fromLTRB(
      0,
      0,
      paragraph.maxIntrinsicWidth + 16,
      2 * paragraph.height + 32,
    );
    final Rect outerClip =
        Rect.fromLTRB(0.5, 0.5, canvasSize.right, canvasSize.bottom);
    final Rect innerClip = Rect.fromLTRB(0.5, canvasSize.bottom / 2 + 0.5,
        canvasSize.right, canvasSize.bottom);

    canvas = BitmapCanvas(canvasSize, RenderStrategy());
    canvas.debugChildOverdraw = true;
    canvas.clipRect(outerClip, ClipOp.intersect);
    canvas.drawParagraph(paragraph, const Offset(8.5, 8.5));
    canvas.clipRect(innerClip, ClipOp.intersect);
    canvas.drawParagraph(paragraph, Offset(8.5, 8.5 + innerClip.top));

    expect(
      canvas.rootElement.querySelectorAll('p').map<String>((html.Element e) => e.innerText).toList(),
      <String>['Am I blurry?', 'Am I blurry?'],
      reason: 'Expected to render text using HTML',
    );

    appendToScene();

    await matchGoldenFile(
      'bitmap_canvas_draws_high_quality_text.png',
      region: canvasSize,
      maxDiffRatePercent: 0.0,
      pixelComparison: PixelComparison.precise,
    );
  }, testOn: 'chrome');

  // NOTE: Chrome in --headless mode does not reproduce the bug that this test
  //       attempts to reproduce. However, it's still good to have this test
  //       for potential future regressions related to paint order.
  test('draws text on top of canvas when transformed and clipped', () async {
    final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle(
      fontFamily: 'Ahem',
      fontSize: 18,
    ));

    const String text = 'This text is intentionally very long to make sure that it '
      'breaks into multiple lines.';
    builder.addText(text);

    final EngineParagraph paragraph = builder.build() as EngineParagraph;
    paragraph.layout(const ParagraphConstraints(width: 100));

    final Rect canvasSize = Offset.zero & Size(500, 500);

    canvas = BitmapCanvas(canvasSize, RenderStrategy());
    canvas.debugChildOverdraw = true;

    final SurfacePaintData pathPaint = SurfacePaintData()
      ..color = const Color(0xFF7F7F7F)
      ..style = PaintingStyle.fill;

    const double r = 200.0;
    const double l = 50.0;

    final Path path = (Path()
      ..moveTo(-l, -l)
      ..lineTo(0, -r)
      ..lineTo(l, -l)
      ..lineTo(r, 0)
      ..lineTo(l, l)
      ..lineTo(0, r)
      ..lineTo(-l, l)
      ..lineTo(-r, 0)
      ..close()).shift(const Offset(250, 250));

    canvas.drawPath(path, pathPaint);
    canvas.drawParagraph(paragraph, const Offset(180, 50));

    expect(
      canvas.rootElement.querySelectorAll('p').map<String?>((html.Element e) => e.text).toList(),
      <String>[text],
      reason: 'Expected to render text using HTML',
    );

    final SceneBuilder sb = SceneBuilder();
    sb.pushTransform(Matrix4.diagonal3Values(EnginePlatformDispatcher.browserDevicePixelRatio,
        EnginePlatformDispatcher.browserDevicePixelRatio, 1.0).toFloat64());
    sb.pushTransform(Matrix4.rotationZ(math.pi / 2).toFloat64());
    sb.pushOffset(0, -500);
    sb.pushClipRect(canvasSize);
    sb.pop();
    sb.pop();
    sb.pop();
    sb.pop();
    final SurfaceScene scene = sb.build() as SurfaceScene;
    final html.Element sceneElement = scene.webOnlyRootElement!;

    sceneElement.querySelector('flt-clip')!.append(canvas.rootElement);
    domRenderer.glassPaneShadow!.querySelector('flt-scene-host')!.append(sceneElement);

    await matchGoldenFile(
      'bitmap_canvas_draws_text_on_top_of_canvas.png',
      region: canvasSize,
      maxDiffRatePercent: 1.0,
      pixelComparison: PixelComparison.precise,
    );
  });
}
