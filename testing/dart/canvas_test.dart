// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';
import 'dart:ui';

import 'package:litetest/litetest.dart';
import 'package:path/path.dart' as path;
import 'package:vector_math/vector_math_64.dart';

import 'goldens.dart';
import 'impeller_enabled.dart';

typedef CanvasCallback = void Function(Canvas canvas);

Future<Image> createImage(int width, int height) {
  final Completer<Image> completer = Completer<Image>();
  decodeImageFromPixels(
    Uint8List.fromList(List<int>.generate(
      width * height * 4,
      (int pixel) => pixel % 255,
    )),
    width,
    height,
    PixelFormat.rgba8888,
    (Image image) {
      completer.complete(image);
    },
  );

  return completer.future;
}

void testCanvas(CanvasCallback callback) {
  try {
    callback(Canvas(PictureRecorder(), Rect.zero));
  } catch (error) { } // ignore: empty_catches
}

Future<Image> toImage(CanvasCallback callback, int width, int height) {
  final PictureRecorder recorder = PictureRecorder();
  final Canvas canvas = Canvas(recorder, Rect.fromLTRB(0, 0, width.toDouble(), height.toDouble()));
  callback(canvas);
  final Picture picture = recorder.endRecording();
  return picture.toImage(width, height);
}

void testNoCrashes() {
  test('canvas APIs should not crash', () async {
    final Paint paint = Paint();
    const Rect rect = Rect.fromLTRB(double.nan, double.nan, double.nan, double.nan);
    final RRect rrect = RRect.fromRectAndCorners(rect);
    const Offset offset = Offset(double.nan, double.nan);
    final Path path = Path();
    const Color color = Color(0x00000000);
    final Paragraph paragraph = ParagraphBuilder(ParagraphStyle()).build();

    final PictureRecorder recorder = PictureRecorder();
    final Canvas recorderCanvas = Canvas(recorder);
    recorderCanvas.scale(1.0, 1.0);
    final Picture picture = recorder.endRecording();
    final Image image = await picture.toImage(1, 1);

    try { Canvas(PictureRecorder()); } catch (error) { } // ignore: empty_catches
    try { Canvas(PictureRecorder(), rect); } catch (error) { } // ignore: empty_catches

    try {
      PictureRecorder()
        ..endRecording()
        ..endRecording()
        ..endRecording();
    } catch (error) { } // ignore: empty_catches

    testCanvas((Canvas canvas) => canvas.clipPath(path));
    testCanvas((Canvas canvas) => canvas.clipRect(rect));
    testCanvas((Canvas canvas) => canvas.clipRRect(rrect));
    testCanvas((Canvas canvas) => canvas.drawArc(rect, 0.0, 0.0, false, paint));
    testCanvas((Canvas canvas) => canvas.drawAtlas(image, <RSTransform>[], <Rect>[], <Color>[], BlendMode.src, rect, paint));
    testCanvas((Canvas canvas) => canvas.drawCircle(offset, double.nan, paint));
    testCanvas((Canvas canvas) => canvas.drawColor(color, BlendMode.src));
    testCanvas((Canvas canvas) => canvas.drawDRRect(rrect, rrect, paint));
    testCanvas((Canvas canvas) => canvas.drawImage(image, offset, paint));
    testCanvas((Canvas canvas) => canvas.drawImageNine(image, rect, rect, paint));
    testCanvas((Canvas canvas) => canvas.drawImageRect(image, rect, rect, paint));
    testCanvas((Canvas canvas) => canvas.drawLine(offset, offset, paint));
    testCanvas((Canvas canvas) => canvas.drawOval(rect, paint));
    testCanvas((Canvas canvas) => canvas.drawPaint(paint));
    testCanvas((Canvas canvas) => canvas.drawParagraph(paragraph, offset));
    testCanvas((Canvas canvas) => canvas.drawPath(path, paint));
    testCanvas((Canvas canvas) => canvas.drawPicture(picture));
    testCanvas((Canvas canvas) => canvas.drawPoints(PointMode.points, <Offset>[], paint));
    testCanvas((Canvas canvas) => canvas.drawRawAtlas(image, Float32List(0), Float32List(0), Int32List(0), BlendMode.src, rect, paint));
    testCanvas((Canvas canvas) => canvas.drawRawPoints(PointMode.points, Float32List(0), paint));
    testCanvas((Canvas canvas) => canvas.drawRect(rect, paint));
    testCanvas((Canvas canvas) => canvas.drawRRect(rrect, paint));
    testCanvas((Canvas canvas) => canvas.drawShadow(path, color, double.nan, false));
    testCanvas((Canvas canvas) => canvas.drawShadow(path, color, double.nan, true));
    testCanvas((Canvas canvas) => canvas.drawVertices(Vertices(VertexMode.triangles, <Offset>[]), BlendMode.screen, paint));
    testCanvas((Canvas canvas) => canvas.getSaveCount());
    testCanvas((Canvas canvas) => canvas.restore());
    testCanvas((Canvas canvas) => canvas.rotate(double.nan));
    testCanvas((Canvas canvas) => canvas.save());
    testCanvas((Canvas canvas) => canvas.saveLayer(rect, paint));
    testCanvas((Canvas canvas) => canvas.saveLayer(null, paint));
    testCanvas((Canvas canvas) => canvas.scale(double.nan, double.nan));
    testCanvas((Canvas canvas) => canvas.skew(double.nan, double.nan));
    testCanvas((Canvas canvas) => canvas.transform(Float64List(16)));
    testCanvas((Canvas canvas) => canvas.translate(double.nan, double.nan));
    testCanvas((Canvas canvas) => canvas.drawVertices(Vertices(VertexMode.triangles, <Offset>[],
                                                               indices: <int>[]), BlendMode.screen, paint));
    testCanvas((Canvas canvas) => canvas.drawVertices(Vertices(VertexMode.triangles, <Offset>[])..dispose(), BlendMode.screen, paint));

    // Regression test for https://github.com/flutter/flutter/issues/115143
    testCanvas((Canvas canvas) => canvas.drawPaint(Paint()..imageFilter = const ColorFilter.mode(Color(0x00000000), BlendMode.xor)));

    // Regression test for https://github.com/flutter/flutter/issues/120278
    testCanvas((Canvas canvas) => canvas.drawPaint(Paint()..imageFilter = ImageFilter.compose(
      outer: ImageFilter.matrix(Matrix4.identity().storage),
      inner: ImageFilter.blur())));
  });
}

const String kFlutterBuildDirectory = 'kFlutterBuildDirectory';

String get _flutterBuildPath {
  const String buildPath = String.fromEnvironment(kFlutterBuildDirectory);
  if (buildPath.isEmpty) {
    throw StateError('kFlutterBuildDirectory -D variable is not set.');
  }
  return buildPath;
}

void main() async {
  final ImageComparer comparer = await ImageComparer.create();


  test('toImage and toImageSync have identical contents', () async {
    // Note: on linux this stil seems to be different.
    // TODO(jonahwilliams): https://github.com/flutter/flutter/issues/108835
    if (Platform.isLinux) {
      return;
    }

    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawRect(
      const Rect.fromLTWH(20, 20, 100, 100),
      Paint()..color = const Color(0xA0FF6D00),
    );
    final Picture picture = recorder.endRecording();
    final Image toImageImage = await picture.toImage(200, 200);
    final Image toImageSyncImage = picture.toImageSync(200, 200);

    // To trigger observable difference in alpha, draw image
    // on a second canvas.
    Future<ByteData> drawOnCanvas(Image image) async {
      final PictureRecorder recorder = PictureRecorder();
      final Canvas canvas = Canvas(recorder);
      canvas.drawPaint(Paint()..color = const Color(0x4FFFFFFF));
      canvas.drawImage(image, Offset.zero, Paint());
      final Image resultImage = await recorder.endRecording().toImage(200, 200);
      sleep(const Duration(milliseconds: 300));
      return (await resultImage.toByteData())!;
    }

    final ByteData dataSync = await drawOnCanvas(toImageImage);
    final ByteData data = await drawOnCanvas(toImageSyncImage);
    expect(data, listEquals(dataSync));
  });

  test('Canvas.drawParagraph throws when Paragraph.layout was not called', () async {
    // Regression test for https://github.com/flutter/flutter/issues/97172
    bool assertsEnabled = false;
    assert(() {
      assertsEnabled = true;
      return true;
    }());

    Object? error;
    try {
      await toImage((Canvas canvas) {
        final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle());
        builder.addText('Woodstock!');
        final Paragraph woodstock = builder.build();
        canvas.drawParagraph(woodstock, const Offset(0, 50));
      }, 100, 100);
    } catch (e) {
      error = e;
    }
    if (assertsEnabled) {
      expect(error, isNotNull);
    } else {
      expect(error, isNull);
    }
  });

  Future<Image> drawText(String text) {
    return toImage((Canvas canvas) {
      final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle(
        fontFamily: 'RobotoSerif',
        fontStyle: FontStyle.normal,
        fontWeight: FontWeight.normal,
        fontSize: 15.0,
      ));
      builder.pushStyle(TextStyle(color: const Color(0xFF0000FF)));
      builder.addText(text);

      final Paragraph paragraph = builder.build();
      paragraph.layout(const ParagraphConstraints(width: 20 * 5.0));

      canvas.drawParagraph(paragraph, Offset.zero);
    }, 100, 100);
  }

  test('Canvas.drawParagraph renders tab as space instead of tofu', () async {
    // Skia renders a tofu if the font does not have a glyph for a character.
    // However, Flutter opts-in to a Skia feature to render tabs as a single space.
    // See: https://github.com/flutter/flutter/issues/79153
    final File file = File(path.join(_flutterBuildPath, 'flutter', 'third_party', 'txt', 'assets', 'Roboto-Regular.ttf'));
    final Uint8List fontData = await file.readAsBytes();
    await loadFontFromList(fontData, fontFamily: 'RobotoSerif');

    // The backspace character, \b, does not have a corresponding glyph and is rendered as a tofu.
    final Image tabImage = await drawText('>\t<');
    final Image spaceImage = await drawText('> <');
    final Image tofuImage = await drawText('>\b<');

    // The tab's image should be identical to the space's image but not the tofu's image.
    final bool tabToSpaceComparison = await comparer.fuzzyCompareImages(tabImage, spaceImage);
    final bool tabToTofuComparison = await comparer.fuzzyCompareImages(tabImage, tofuImage);

    expect(tabToSpaceComparison, isTrue);
    expect(tabToTofuComparison, isFalse);
  });

  test('drawRect, drawOval, and clipRect render with unsorted rectangles', () async {
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);

    canvas.drawColor(const Color(0xFFE0E0E0), BlendMode.src);

    void draw(Rect rect, double x, double y, Color color) {
      final Paint paint = Paint()
        ..color = color
        ..strokeWidth = 5.0;

      final Rect tallThin = Rect.fromLTRB(
        min(rect.left, rect.right) - 10,
        rect.top,
        min(rect.left, rect.right) - 10,
        rect.bottom,
      );
      final Rect wideThin = Rect.fromLTRB(
        rect.left,
        min(rect.top, rect.bottom) - 10,
        rect.right,
        min(rect.top, rect.bottom) - 10,
      );

      canvas.save();
      canvas.translate(x, y);

      paint.style = PaintingStyle.fill;
      canvas.drawRect(rect, paint);
      canvas.drawRect(tallThin, paint);
      canvas.drawRect(wideThin, paint);

      canvas.save();
      canvas.translate(0, 100);
      paint.style = PaintingStyle.stroke;
      canvas.drawRect(rect, paint);
      canvas.drawRect(tallThin, paint);
      canvas.drawRect(wideThin, paint);
      canvas.restore();

      canvas.save();
      canvas.translate(100, 0);
      paint.style = PaintingStyle.fill;
      canvas.drawOval(rect, paint);
      canvas.drawOval(tallThin, paint);
      canvas.drawOval(wideThin, paint);
      canvas.restore();

      canvas.save();
      canvas.translate(100, 100);
      paint.style = PaintingStyle.stroke;
      canvas.drawOval(rect, paint);
      canvas.drawOval(tallThin, paint);
      canvas.drawOval(wideThin, paint);
      canvas.restore();

      canvas.save();
      canvas.translate(50, 50);

      canvas.save();
      canvas.clipRect(rect);
      canvas.drawPaint(paint);
      canvas.restore();

      canvas.save();
      canvas.clipRect(tallThin);
      canvas.drawPaint(paint);
      canvas.restore();

      canvas.save();
      canvas.clipRect(wideThin);
      canvas.drawPaint(paint);
      canvas.restore();

      canvas.restore();

      canvas.restore();
    }

    draw(const Rect.fromLTRB(10, 10, 40, 40), 50, 50, const Color(0xFF2196F3));
    draw(const Rect.fromLTRB(40, 10, 10, 40), 250, 50, const Color(0xFF4CAF50));
    draw(const Rect.fromLTRB(10, 40, 40, 10), 50, 250, const Color(0xFF9C27B0));
    draw(const Rect.fromLTRB(40, 40, 10, 10), 250, 250, const Color(0xFFFF9800));

    final Picture picture = recorder.endRecording();
    final Image image = await picture.toImage(450, 450);
    await comparer.addGoldenImage(image, 'render_unordered_rects.png');
  });

  Matcher closeToTransform(Float64List expected) => (dynamic v) {
    Expect.type<Float64List>(v);
    final Float64List value = v as Float64List;
    expect(expected.length, equals(16));
    expect(value.length, equals(16));
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        final double vActual = value[r*4 + c];
        final double vExpected = expected[r*4 + c];
        if ((vActual - vExpected).abs() > 1e-10) {
          Expect.fail('matrix mismatch at $r, $c, $vActual not close to $vExpected');
        }
      }
    }
  };

  Matcher notCloseToTransform(Float64List expected) => (dynamic v) {
    Expect.type<Float64List>(v);
    final Float64List value = v as Float64List;
    expect(expected.length, equals(16));
    expect(value.length, equals(16));
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        final double vActual = value[r*4 + c];
        final double vExpected = expected[r*4 + c];
        if ((vActual - vExpected).abs() > 1e-10) {
          return;
        }
      }
    }
    Expect.fail('$value is too close to $expected');
  };
}

Matcher listEquals(ByteData expected) => (dynamic v) {
  Expect.type<ByteData>(v);
  final ByteData value = v as ByteData;
  expect(value.lengthInBytes, expected.lengthInBytes);
  for (int i = 0; i < value.lengthInBytes; i++) {
    expect(value.getUint8(i), expected.getUint8(i));
  }
};
