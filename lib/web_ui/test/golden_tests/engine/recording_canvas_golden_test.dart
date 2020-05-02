// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:html' as html;
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:ui/ui.dart' hide TextStyle;
import 'package:ui/src/engine.dart';
import 'package:test/test.dart';

import '../../matchers.dart';
import 'package:web_engine_tester/golden_tester.dart';

void main() async {
  const double screenWidth = 600.0;
  const double screenHeight = 800.0;
  const Rect screenRect = Rect.fromLTWH(0, 0, screenWidth, screenHeight);
  final Paint testPaint = Paint()..color = const Color(0xFFFF0000);

  // Commit a recording canvas to a bitmap, and compare with the expected
  Future<void> _checkScreenshot(RecordingCanvas rc, String fileName,
      { Rect region = const Rect.fromLTWH(0, 0, 500, 500) }) async {

    final EngineCanvas engineCanvas = BitmapCanvas(screenRect);

    // Draws the estimated bounds so we can spot the bug in Scuba.
    engineCanvas
      ..save()
      ..drawRect(
        rc.pictureBounds,
        SurfacePaintData()
          ..color = const Color.fromRGBO(0, 0, 255, 1.0)
          ..style = PaintingStyle.stroke
          ..strokeWidth = 1.0,
      )
      ..restore();

    rc.apply(engineCanvas, screenRect);

    // Wrap in <flt-scene> so that our CSS selectors kick in.
    final html.Element sceneElement = html.Element.tag('flt-scene');
    try {
      sceneElement.append(engineCanvas.rootElement);
      html.document.body.append(sceneElement);
      await matchGoldenFile('paint_bounds_for_$fileName.png', region: region);
    } finally {
      // The page is reused across tests, so remove the element after taking the
      // Scuba screenshot.
      sceneElement.remove();
    }
  }

  setUp(() async {
    debugEmulateFlutterTesterEnvironment = true;
    await webOnlyInitializePlatform();
    webOnlyFontCollection.debugRegisterTestFonts();
    await webOnlyFontCollection.ensureFontsLoaded();
  });

  test('Empty canvas reports correct paint bounds', () async {
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTWH(1, 2, 300, 400));
    rc.endRecording();
    expect(rc.pictureBounds, Rect.zero);
    await _checkScreenshot(rc, 'empty_canvas');
  });

  test('Computes paint bounds for draw line', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawLine(const Offset(50, 100), const Offset(120, 140), testPaint);
    rc.endRecording();
    // The off by one is due to the minimum stroke width of 1.
    expect(rc.pictureBounds, const Rect.fromLTRB(49, 99, 121, 141));
    await _checkScreenshot(rc, 'draw_line');
  });

  test('Computes paint bounds for draw line when line exceeds limits',
      () async {
    // Uses max bounds when computing paint bounds
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawLine(const Offset(50, 100), const Offset(screenWidth + 100.0, 140),
        testPaint);
    rc.endRecording();
    // The off by one is due to the minimum stroke width of 1.
    expect(rc.pictureBounds,
        const Rect.fromLTRB(49.0, 99.0, screenWidth, 141.0));
    await _checkScreenshot(rc, 'draw_line_exceeding_limits');
  });

  test('Computes paint bounds for draw rect', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawRect(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(10, 20, 30, 40));
    await _checkScreenshot(rc, 'draw_rect');
  });

  test('Computes paint bounds for draw rect when exceeds limits', () async {
    // Uses max bounds when computing paint bounds
    RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawRect(
        const Rect.fromLTRB(10, 20, 30 + screenWidth, 40 + screenHeight),
        testPaint);
    rc.endRecording();
    expect(rc.pictureBounds,
        const Rect.fromLTRB(10, 20, screenWidth, screenHeight));

    rc = RecordingCanvas(screenRect);
    rc.drawRect(const Rect.fromLTRB(-200, -100, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(0, 0, 30, 40));
    await _checkScreenshot(rc, 'draw_rect_exceeding_limits');
  });

  test('Computes paint bounds for translate', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.translate(5, 7);
    rc.drawRect(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(15, 27, 35, 47));
    await _checkScreenshot(rc, 'translate');
  });

  test('Computes paint bounds for scale', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.scale(2, 2);
    rc.drawRect(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(20, 40, 60, 80));
    await _checkScreenshot(rc, 'scale');
  });

  test('Computes paint bounds for rotate', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.rotate(math.pi / 4.0);
    rc.drawLine(
        const Offset(1, 0), Offset(50 * math.sqrt(2) - 1, 0), testPaint);
    rc.endRecording();
    // The extra 0.7 is due to stroke width of 1 rotated by 45 degrees.
    expect(rc.pictureBounds,
        within(distance: 0.1, from: const Rect.fromLTRB(0, 0, 50.7, 50.7)));
    await _checkScreenshot(rc, 'rotate');
  });

  test('Computes paint bounds for horizontal skew', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.skew(1.0, 0.0);
    rc.drawRect(const Rect.fromLTRB(20, 20, 40, 40), testPaint);
    rc.endRecording();
    expect(
        rc.pictureBounds,
        within(
            distance: 0.1, from: const Rect.fromLTRB(40.0, 20.0, 80.0, 40.0)));
    await _checkScreenshot(rc, 'skew_horizontally');
  });

  test('Computes paint bounds for vertical skew', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.skew(0.0, 1.0);
    rc.drawRect(const Rect.fromLTRB(20, 20, 40, 40), testPaint);
    rc.endRecording();
    expect(
        rc.pictureBounds,
        within(
            distance: 0.1, from: const Rect.fromLTRB(20.0, 40.0, 40.0, 80.0)));
    await _checkScreenshot(rc, 'skew_vertically');
  });

  test('Computes paint bounds for a complex transform', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    final Float32List matrix = Float32List(16);
    // translate(210, 220) , scale(2, 3), rotate(math.pi / 4.0)
    matrix[0] = 1.4;
    matrix[1] = 2.12;
    matrix[2] = 0.0;
    matrix[3] = 0.0;
    matrix[4] = -1.4;
    matrix[5] = 2.12;
    matrix[6] = 0.0;
    matrix[7] = 0.0;
    matrix[8] = 0.0;
    matrix[9] = 0.0;
    matrix[10] = 2.0;
    matrix[11] = 0.0;
    matrix[12] = 210.0;
    matrix[13] = 220.0;
    matrix[14] = 0.0;
    matrix[15] = 1.0;
    rc.transform(matrix);
    rc.drawRect(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds,
        within(distance: 0.001, from: const Rect.fromLTRB(168.0, 283.6, 224.0, 368.4)));
    await _checkScreenshot(rc, 'complex_transform');
  });

  test('drawPaint should cover full size', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawPaint(testPaint);
    rc.drawRect(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, screenRect);
    await _checkScreenshot(rc, 'draw_paint');
  });

  test('drawColor should cover full size', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    final Paint testPaint = Paint()..color = const Color(0xFF80FF00);
    rc.drawRect(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.drawColor(const Color(0xFFFF0000), BlendMode.multiply);
    rc.drawRect(const Rect.fromLTRB(10, 60, 30, 80), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, screenRect);
    await _checkScreenshot(rc, 'draw_color');
  });

  test('Computes paint bounds for draw oval', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawOval(const Rect.fromLTRB(10, 20, 30, 40), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(10, 20, 30, 40));
    await _checkScreenshot(rc, 'draw_oval');
  });

  test('Computes paint bounds for draw round rect', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawRRect(
        RRect.fromRectAndRadius(
            const Rect.fromLTRB(10, 20, 30, 40), const Radius.circular(5.0)),
        testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(10, 20, 30, 40));
    await _checkScreenshot(rc, 'draw_round_rect');
  });

  test(
      'Computes empty paint bounds when inner rect outside of outer rect for '
      'drawDRRect', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawDRRect(RRect.fromRectAndCorners(const Rect.fromLTRB(10, 20, 30, 40)),
        RRect.fromRectAndCorners(const Rect.fromLTRB(1, 2, 3, 4)), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(0, 0, 0, 0));
    await _checkScreenshot(rc, 'draw_drrect_empty');
  });

  test('Computes paint bounds using outer rect for drawDRRect', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawDRRect(
        RRect.fromRectAndCorners(const Rect.fromLTRB(10, 20, 30, 40)),
        RRect.fromRectAndCorners(const Rect.fromLTRB(12, 22, 28, 38)),
        testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(10, 20, 30, 40));
    await _checkScreenshot(rc, 'draw_drrect');
  });

  test('Computes paint bounds for draw circle', () async {
    // Paint bounds of one circle.
    RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawCircle(const Offset(20, 20), 10.0, testPaint);
    rc.endRecording();
    expect(
        rc.pictureBounds, const Rect.fromLTRB(10.0, 10.0, 30.0, 30.0));

    // Paint bounds of a union of two circles.
    rc = RecordingCanvas(screenRect);
    rc.drawCircle(const Offset(20, 20), 10.0, testPaint);
    rc.drawCircle(const Offset(200, 300), 100.0, testPaint);
    rc.endRecording();
    expect(
        rc.pictureBounds, const Rect.fromLTRB(10.0, 10.0, 300.0, 400.0));
    await _checkScreenshot(rc, 'draw_circle');
  });

  test('Computes paint bounds for draw image', () {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawImage(TestImage(), const Offset(50, 100), Paint());
    rc.endRecording();
    expect(
        rc.pictureBounds, const Rect.fromLTRB(50.0, 100.0, 70.0, 110.0));
  });

  test('Computes paint bounds for draw image rect', () {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.drawImageRect(TestImage(), const Rect.fromLTRB(1, 1, 20, 10),
        const Rect.fromLTRB(5, 6, 400, 500), Paint());
    rc.endRecording();
    expect(
        rc.pictureBounds, const Rect.fromLTRB(5.0, 6.0, 400.0, 500.0));
  });

  test('Computes paint bounds for single-line draw paragraph', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    final Paragraph paragraph = createTestParagraph();
    const double textLeft = 5.0;
    const double textTop = 7.0;
    const double widthConstraint = 300.0;
    paragraph.layout(const ParagraphConstraints(width: widthConstraint));
    rc.drawParagraph(paragraph, const Offset(textLeft, textTop));
    rc.endRecording();
    expect(
      rc.pictureBounds,
      const Rect.fromLTRB(textLeft, textTop, textLeft + widthConstraint, 21.0),
    );
    await _checkScreenshot(rc, 'draw_paragraph');
  });

  test('Computes paint bounds for multi-line draw paragraph', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    final Paragraph paragraph = createTestParagraph();
    const double textLeft = 5.0;
    const double textTop = 7.0;
    // Do not go lower than the shortest word.
    const double widthConstraint = 130.0;
    paragraph.layout(const ParagraphConstraints(width: widthConstraint));
    rc.drawParagraph(paragraph, const Offset(textLeft, textTop));
    rc.endRecording();
    expect(
      rc.pictureBounds,
      const Rect.fromLTRB(textLeft, textTop, textLeft + widthConstraint, 35.0),
    );
    await _checkScreenshot(rc, 'draw_paragraph_multi_line');
  });

  test('Should exclude painting outside simple clipRect', () async {
    // One clipped line.
    RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.clipRect(const Rect.fromLTRB(50, 50, 100, 100));
    rc.drawLine(const Offset(10, 11), const Offset(20, 21), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, Rect.zero);

    // Two clipped lines.
    rc = RecordingCanvas(screenRect);
    rc.clipRect(const Rect.fromLTRB(50, 50, 100, 100));
    rc.drawLine(const Offset(10, 11), const Offset(20, 21), testPaint);
    rc.drawLine(const Offset(52, 53), const Offset(55, 56), testPaint);
    rc.endRecording();

    // Extra pixel due to default line length
    expect(rc.pictureBounds, const Rect.fromLTRB(51, 52, 56, 57));
    await _checkScreenshot(rc, 'clip_rect_simple');
  });

  test('Should include intersection of clipRect and painting', () async {
    RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.clipRect(const Rect.fromLTRB(50, 50, 100, 100));
    rc.drawRect(const Rect.fromLTRB(20, 60, 120, 70), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(50, 60, 100, 70));
    await _checkScreenshot(rc, 'clip_rect_intersects_paint_left_to_right');

    rc = RecordingCanvas(screenRect);
    rc.clipRect(const Rect.fromLTRB(50, 50, 100, 100));
    rc.drawRect(const Rect.fromLTRB(60, 20, 70, 200), testPaint);
    rc.endRecording();
    expect(rc.pictureBounds, const Rect.fromLTRB(60, 50, 70, 100));
    await _checkScreenshot(rc, 'clip_rect_intersects_paint_top_to_bottom');
  });

  test('Should intersect rects for multiple clipRect calls', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    rc.clipRect(const Rect.fromLTRB(50, 50, 100, 100));
    rc.scale(2.0, 2.0);
    rc.clipRect(const Rect.fromLTRB(30, 30, 45, 45));
    rc.drawRect(const Rect.fromLTRB(10, 30, 60, 35), testPaint);
    rc.endRecording();

    expect(rc.pictureBounds, const Rect.fromLTRB(60, 60, 90, 70));
    await _checkScreenshot(rc, 'clip_rects_intersect');
  });

  // drawShadow
  test('Computes paint bounds for drawShadow', () async {
    final RecordingCanvas rc = RecordingCanvas(screenRect);
    final Path path = Path();
    path.addRect(const Rect.fromLTRB(20, 30, 100, 110));
    rc.drawShadow(path, const Color(0xFFFF0000), 2.0, true);
    rc.endRecording();

    expect(
      rc.pictureBounds,
      within(distance: 0.05, from: const Rect.fromLTRB(17.9, 28.5, 103.5, 114.1)),
    );
    await _checkScreenshot(rc, 'path_with_shadow');
  });

  test('Clip with negative scale reports correct paint bounds', () async {
    // The following draws a filled rectangle that occupies the bottom half of
    // the canvas. Notice that both the clip and the rectangle are drawn
    // forward. What makes them appear at the bottom is the translation and a
    // vertical flip via a negative scale. This replicates the Material
    // overscroll glow effect at the bottom of a list, where it is drawn upside
    // down.
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 100, 100));
    rc
      ..translate(0, 100)
      ..scale(1, -1)
      ..clipRect(const Rect.fromLTRB(0, 0, 100, 50))
      ..drawRect(const Rect.fromLTRB(0, 0, 100, 100), Paint());
    rc.endRecording();

    expect(
        rc.pictureBounds, const Rect.fromLTRB(0.0, 50.0, 100.0, 100.0));
    await _checkScreenshot(rc, 'scale_negative');
  });

  test('Clip with a rotation reports correct paint bounds', () async {
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 100, 100));
    rc
      ..translate(50, 50)
      ..rotate(math.pi / 4.0)
      ..clipRect(const Rect.fromLTWH(-20, -20, 40, 40))
      ..drawRect(const Rect.fromLTWH(-80, -80, 160, 160), Paint());
    rc.endRecording();

    expect(
      rc.pictureBounds,
      within(distance: 0.001, from: Rect.fromCircle(center: const Offset(50, 50), radius: 20 * math.sqrt(2))),
    );
    await _checkScreenshot(rc, 'clip_rect_rotated');
  });

  test('Rotated line reports correct paint bounds', () async {
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 100, 100));
    rc
      ..translate(50, 50)
      ..rotate(math.pi / 4.0)
      ..drawLine(const Offset(0, 0), const Offset(20, 20), Paint());
    rc.endRecording();

    expect(
      rc.pictureBounds,
      within(distance: 0.1, from: const Rect.fromLTRB(34.4, 48.6, 65.6, 79.7)),
    );
    await _checkScreenshot(rc, 'line_rotated');
  });

  test('Should support reusing path and reset when drawing into canvas.',
      () async {
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 100, 100));

    final Path path = Path();
    path.moveTo(3, 0);
    path.lineTo(100, 97);
    rc.drawPath(
        path,
        Paint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 2.0
          ..color = const Color(0xFFFF0000));
    path.reset();
    path.moveTo(0, 3);
    path.lineTo(97, 100);
    rc.drawPath(
        path,
        Paint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 2.0
          ..color = const Color(0xFF00FF00));
    rc.endRecording();
    await _checkScreenshot(rc, 'reuse_path');
  });

  test('Should draw RRect after line when beginning new path.', () async {
    final RecordingCanvas rc =
        RecordingCanvas(const Rect.fromLTRB(0, 0, 200, 400));
    rc.save();
    rc.translate(50.0, 100.0);
    final Path path = Path();
    // Draw a vertical small line (caret).
    path.moveTo(8, 4);
    path.lineTo(8, 24);
    // Draw round rect below caret.
    path.addRRect(
        RRect.fromLTRBR(0.5, 100.5, 80.7, 150.7, const Radius.circular(10)));
    rc.drawPath(
        path,
        Paint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 2.0
          ..color = const Color(0xFF404000));
    rc.restore();
    rc.endRecording();
    await _checkScreenshot(rc, 'path_with_line_and_roundrect');
  });

  test('should include paint spread in bounds estimates', () async {
    final SurfaceSceneBuilder sb = SurfaceSceneBuilder();

    final List<PaintSpreadPainter> painters = <PaintSpreadPainter>[
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawLine(
          const Offset(0.0, 0.0),
          const Offset(20.0, 20.0),
          paint,
        );
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawRect(
          const Rect.fromLTRB(0.0, 0.0, 20.0, 20.0),
          paint,
        );
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawRRect(
          RRect.fromLTRBR(0.0, 0.0, 20.0, 20.0, Radius.circular(7.0)),
          paint,
        );
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawDRRect(
          RRect.fromLTRBR(0.0, 0.0, 20.0, 20.0, Radius.circular(5.0)),
          RRect.fromLTRBR(4.0, 4.0, 16.0, 16.0, Radius.circular(5.0)),
          paint,
        );
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawOval(
          const Rect.fromLTRB(0.0, 5.0, 20.0, 15.0),
          paint,
        );
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawCircle(
          const Offset(10.0, 10.0),
          10.0,
          paint,
        );
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        final SurfacePath path = SurfacePath()
          ..moveTo(10, 0)
          ..lineTo(20, 10)
          ..lineTo(10, 20)
          ..lineTo(0, 10)
          ..close();
        canvas.drawPath(path, paint);
      },

      // Images are not affected by mask filter or stroke width. They use image
      // filter instead.
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawImage(_createRealTestImage(), Offset.zero, paint);
      },
      (RecordingCanvas canvas, SurfacePaint paint) {
        canvas.drawImageRect(
          _createRealTestImage(),
          const Rect.fromLTRB(0, 0, 20, 20),
          const Rect.fromLTRB(5, 5, 15, 15),
          paint,
        );
      },
    ];

    Picture drawBounds(Rect bounds) {
      final EnginePictureRecorder recorder = EnginePictureRecorder();
      final RecordingCanvas canvas = recorder.beginRecording(Rect.largest);
      canvas.drawRect(
        bounds,
        SurfacePaint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 1.0
          ..color = const Color.fromARGB(255, 0, 255, 0),
      );
      return recorder.endRecording();
    }

    for (int i = 0; i < painters.length; i++) {
      sb.pushOffset(0.0, 20.0 + 60.0 * i);
      final PaintSpreadPainter painter = painters[i];

      // Paint with zero paint spread.
      {
        sb.pushOffset(20.0, 0.0);
        final EnginePictureRecorder recorder = EnginePictureRecorder();
        final RecordingCanvas canvas = recorder.beginRecording(Rect.largest);
        final SurfacePaint zeroSpreadPaint = SurfacePaint();
        painter(canvas, zeroSpreadPaint);
        sb.addPicture(Offset.zero, recorder.endRecording());
        sb.addPicture(Offset.zero, drawBounds(canvas.pictureBounds));
        sb.pop();
      }

      // Paint with a thick stroke paint.
      {
        sb.pushOffset(80.0, 0.0);
        final EnginePictureRecorder recorder = EnginePictureRecorder();
        final RecordingCanvas canvas = recorder.beginRecording(Rect.largest);
        final SurfacePaint thickStrokePaint = SurfacePaint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 5.0;
        painter(canvas, thickStrokePaint);
        sb.addPicture(Offset.zero, recorder.endRecording());
        sb.addPicture(Offset.zero, drawBounds(canvas.pictureBounds));
        sb.pop();
      }

      // Paint with a mask filter blur.
      {
        sb.pushOffset(140.0, 0.0);
        final EnginePictureRecorder recorder = EnginePictureRecorder();
        final RecordingCanvas canvas = recorder.beginRecording(Rect.largest);
        final SurfacePaint maskFilterBlurPaint = SurfacePaint()
          ..maskFilter = const MaskFilter.blur(BlurStyle.normal, 5.0);
        painter(canvas, maskFilterBlurPaint);
        sb.addPicture(Offset.zero, recorder.endRecording());
        sb.addPicture(Offset.zero, drawBounds(canvas.pictureBounds));
        sb.pop();
      }

      // Paint with a thick stroke paint and a mask filter blur.
      {
        sb.pushOffset(200.0, 0.0);
        final EnginePictureRecorder recorder = EnginePictureRecorder();
        final RecordingCanvas canvas = recorder.beginRecording(Rect.largest);
        final SurfacePaint thickStrokeAndBlurPaint = SurfacePaint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 5.0
          ..maskFilter = const MaskFilter.blur(BlurStyle.normal, 5.0);
        painter(canvas, thickStrokeAndBlurPaint);
        sb.addPicture(Offset.zero, recorder.endRecording());
        sb.addPicture(Offset.zero, drawBounds(canvas.pictureBounds));
        sb.pop();
      }

      sb.pop();
    }

    final html.Element sceneElement = sb.build().webOnlyRootElement;
    html.document.body.append(sceneElement);
    try {
      await matchGoldenFile(
        'paint_spread_bounds.png',
        region: const Rect.fromLTRB(0, 0, 250, 600),
        maxDiffRatePercent: 0.0,
        pixelComparison: PixelComparison.precise,
      );
    } finally {
      sceneElement.remove();
    }
  });
}

typedef PaintSpreadPainter = void Function(RecordingCanvas canvas, SurfacePaint paint);

const String _base64Encoded20x20TestImage = 'iVBORw0KGgoAAAANSUhEUgAAABQAAAAUC'
    'AIAAAAC64paAAAACXBIWXMAAC4jAAAuIwF4pT92AAAA'
  'B3RJTUUH5AMFFBksg4i3gQAAABl0RVh0Q29tbWVudABDcmVhdGVkIHdpdGggR0lNUFeBDhcAAAAj'
  'SURBVDjLY2TAC/7jlWVioACMah4ZmhnxpyHG0QAb1UyZZgBjWAIm/clP0AAAAABJRU5ErkJggg==';

const String _flutterLogoBase64 = 'iVBORw0KGgoAAAANSUhEUgAAASAAAAEQCAYAAAAQ+akt'
    'AAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyRpVFh0WE1MOmNvbS5hZ'
    'G9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek'
    '5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdG'
    's9IkFkb2JlIFhNUCBDb3JlIDUuMy1jMDExIDY2LjE0NTY2MSwgMjAxMi8wMi8wNi0xNDo1Nj'
    'oyNyAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOT'
    'k5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9Ii'
    'IgeG1sbnM6eG1wTU09Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9tbS8iIHhtbG5zOn'
    'N0UmVmPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvc1R5cGUvUmVzb3VyY2VSZWYjIi'
    'B4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iIHhtcE1NOkRvY3VtZW'
    '50SUQ9InhtcC5kaWQ6MzFFNzQxN0E4NDBBMTFFQTk4RThBQjg5NEIyOENFQTciIHhtcE1NO'
    'kluc3RhbmNlSUQ9InhtcC5paWQ6MzFFNzQxNzk4NDBBMTFFQTk4RThBQjg5NEIyOENFQTci'
    'IHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNiAoTWFjaW50b3NoKSI+IDx4'
    'bXBNTTpEZXJpdmVkRnJvbSBzdFJlZjppbnN0YW5jZUlEPSJ4bXAuZGlkOjAxODAxMTc0MDcyM'
    'DY4MTE4MjJBQUI1NDhBQTAzMDNBIiBzdFJlZjpkb2N1bWVudElEPSJ4bXAuZGlkOjAxODAxMT'
    'c0MDcyMDY4MTE4MjJBQUI1NDhBQTAzMDNBIi8+IDwvcmRmOkRlc2NyaXB0aW9uPiA8L3JkZjp'
    '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';

HtmlImage _createRealTestImage() {
  return HtmlImage(
    html.ImageElement()
      ..src = 'data:text/plain;base64,$_base64Encoded20x20TestImage',
    20,
    20,
  );
}

HtmlImage _createTestImageWithAlphaChannel() {
  return HtmlImage(
    html.ImageElement()
      ..src = 'data:text/plain;base64,$_flutterLogoBase64',
    20,
    20,
  );
}

class TestImage implements Image {
  @override
  int get width => 20;

  @override
  int get height => 10;

  @override
  Future<ByteData> toByteData(
      {ImageByteFormat format = ImageByteFormat.rawRgba}) async {
    throw UnsupportedError('Cannot encode test image');
  }

  @override
  String toString() => '[$width\u00D7$height]';

  @override
  void dispose() {}
}

Paragraph createTestParagraph() {
  final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle(
    fontFamily: 'Ahem',
    fontStyle: FontStyle.normal,
    fontWeight: FontWeight.normal,
    fontSize: 14.0,
  ));
  builder.addText('A short sentence.');
  return builder.build();
}
