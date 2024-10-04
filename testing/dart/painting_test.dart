// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';
import 'dart:ui';

import 'package:test/test.dart';
import 'package:vector_math/vector_math_64.dart';

import 'goldens.dart';

typedef CanvasCallback = void Function(Canvas canvas);

void main() {
  test('Vertices checks', () {
    try {
      Vertices(
        VertexMode.triangles,
        const <Offset>[Offset.zero, Offset.zero, Offset.zero],
        indices: Uint16List.fromList(const <int>[0, 2, 5]),
      );
      throw 'Vertices did not throw the expected error.';
    } on ArgumentError catch (e) {
      expect('$e', 'Invalid argument(s): "indices" values must be valid indices in the positions list (i.e. numbers in the range 0..2), but indices[2] is 5, which is too big.');
    }
    Vertices( // This one does not throw.
      VertexMode.triangles,
      const <Offset>[Offset.zero],
    ).dispose();
    Vertices( // This one should not throw.
      VertexMode.triangles,
      const <Offset>[Offset.zero, Offset.zero, Offset.zero],
      indices: Uint16List.fromList(const <int>[0, 2, 1, 2, 0, 1, 2, 0]), // Uint16List implements List<int> so this is ok.
    ).dispose();
  });

  test('Vertices.raw checks', () {
    expect(() {
      Vertices.raw(
        VertexMode.triangles,
        Float32List.fromList(const <double>[0.0]),
      );
    }, throwsA(isA<ArgumentError>().having((ArgumentError e) => '$e', 'message', 'Invalid argument(s): "positions" must have an even number of entries (each coordinate is an x,y pair).')));

    Object? indicesError;
    try {
      Vertices.raw(
        VertexMode.triangles,
        Float32List.fromList(const <double>[0.0, 0.0, 0.0, 0.0, 0.0, 0.0]),
        indices: Uint16List.fromList(const <int>[0, 2, 5]),
      );
      throw 'Vertices.raw did not throw the expected error.';
    } on ArgumentError catch (e) {
      indicesError = e;
    }
    expect('$indicesError', 'Invalid argument(s): "indices" values must be valid indices in the positions list (i.e. numbers in the range 0..2), but indices[2] is 5, which is too big.');

    Vertices.raw( // This one does not throw.
      VertexMode.triangles,
      Float32List.fromList(const <double>[0.0, 0.0]),
    ).dispose();
    Vertices.raw( // This one should not throw.
      VertexMode.triangles,
      Float32List.fromList(const <double>[0.0, 0.0, 0.0, 0.0, 0.0, 0.0]),
      indices: Uint16List.fromList(const <int>[0, 2, 1, 2, 0, 1, 2, 0]),
    ).dispose();
  });

  test('BackdropFilter with multiple clips', () async {
    // Regression test for https://github.com/flutter/flutter/issues/144211
    Picture makePicture(CanvasCallback callback) {
      final PictureRecorder recorder = PictureRecorder();
      final Canvas canvas = Canvas(recorder);
      callback(canvas);
      return recorder.endRecording();
    }
    final SceneBuilder sceneBuilder = SceneBuilder();

    final Picture redClippedPicture = makePicture((Canvas canvas) {
      canvas.drawPaint(Paint()..color = const Color(0xFFFFFFFF));
      canvas.clipRect(const Rect.fromLTRB(10, 10, 200, 200));
      canvas.clipRect(const Rect.fromLTRB(11, 10, 300, 200));
      canvas.drawPaint(Paint()..color = const Color(0xFFFF0000));
    });
    sceneBuilder.addPicture(Offset.zero, redClippedPicture);

    final Float64List matrix = Float64List(16);
    sceneBuilder.pushBackdropFilter(ImageFilter.matrix(matrix));

    final Picture whitePicture = makePicture((Canvas canvas) {
      canvas.drawPaint(Paint()..color = const Color(0xFFFFFFFF));
    });
    sceneBuilder.addPicture(Offset.zero, whitePicture);

    final Scene scene = sceneBuilder.build();
    final Image image = scene.toImageSync(20, 20);

    final ByteData data = (await image.toByteData())!;
    expect(data.buffer.asUint32List().length, 20 * 20);
    // If clipping went wrong as in the linked issue, there will be red pixels.
    for (final int color in  data.buffer.asUint32List()) {
      expect(color, 0xFFFFFFFF);
    }

    scene.dispose();
    image.dispose();
    whitePicture.dispose();
    redClippedPicture.dispose();
  });

  Image BackdropBlurWithTileMode(TileMode tileMode) {
    Picture makePicture(CanvasCallback callback) {
      final PictureRecorder recorder = PictureRecorder();
      final Canvas canvas = Canvas(recorder);
      callback(canvas);
      return recorder.endRecording();
    }
    final SceneBuilder sceneBuilder = SceneBuilder();

    final Picture blueGreenGridPicture = makePicture((Canvas canvas) {
      const Color white = const Color(0xFFFFFFFF);
      const Color blue = const Color(0xFF0000FF);
      const Color green = const Color(0xFF00FF00);
      canvas.drawColor(white, BlendMode.src);
      for (int i = 0; i < 100; i++) {
        canvas.drawRect(Rect.fromLTRB(i * 5, 0, i * 5, 1000),
                        Paint()..color = (i & 1) == 0 ? green : blue);
        canvas.drawRect(Rect.fromLTRB(0, i * 5, 1000, i * 5),
                        Paint()..color = (i & 1) == 0 ? blue : green);
      }
    });
    sceneBuilder.addPicture(Offset.zero, blueGreenGridPicture);
    sceneBuilder.pushBackdropFilter(ImageFilter.blur(sigmaX: 10, sigmaY: 10, tileMode: tileMode));

    final Scene scene = sceneBuilder.build();
    final Image image = scene.toImageSync(501, 501);

    scene.dispose();
    blueGreenGridPicture.dispose();

    return image;
  }

  test('BackdropFilter with Blur honors TileMode.decal', () async {
    Image image = BackdropBlurWithTileMode(TileMode.decal);

    final ImageComparer comparer = await ImageComparer.create();
    await comparer.addGoldenImage(image, 'dart_ui_backdrop_filter_blur_decal_tile_mode.png');

    image.dispose();
  });

  test('BackdropFilter with Blur honors TileMode.clamp', () async {
    Image image = BackdropBlurWithTileMode(TileMode.clamp);

    final ImageComparer comparer = await ImageComparer.create();
    await comparer.addGoldenImage(image, 'dart_ui_backdrop_filter_blur_clamp_tile_mode.png');

    image.dispose();
  });

  test('BackdropFilter with Blur honors TileMode.mirror', () async {
    Image image = BackdropBlurWithTileMode(TileMode.mirror);

    final ImageComparer comparer = await ImageComparer.create();
    await comparer.addGoldenImage(image, 'dart_ui_backdrop_filter_blur_mirror_tile_mode.png');

    image.dispose();
  });

  test('BackdropFilter with Blur honors TileMode.repeated', () async {
    Image image = BackdropBlurWithTileMode(TileMode.repeated);

    final ImageComparer comparer = await ImageComparer.create();
    await comparer.addGoldenImage(image, 'dart_ui_backdrop_filter_blur_repeated_tile_mode.png');

    image.dispose();
  });

  test('ImageFilter.matrix defaults to FilterQuality.medium', () {
    final Float64List data = Matrix4.identity().storage;
    expect(
      ImageFilter.matrix(data).toString(),
      'ImageFilter.matrix($data, FilterQuality.medium)',
    );
  });
}
