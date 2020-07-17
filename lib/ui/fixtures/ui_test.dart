// @dart = 2.6
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';
import 'dart:ui';

void main() {}

@pragma('vm:entry-point')
void createVertices() {
  const int uint16max = 65535;

  final Int32List colors = Int32List(uint16max);
  final Float32List coords = Float32List(uint16max * 2);
  final Uint16List indices = Uint16List(uint16max);
  final Float32List positions = Float32List(uint16max * 2);
  colors[0] = const Color(0xFFFF0000).value;
  colors[1] = const Color(0xFF00FF00).value;
  colors[2] = const Color(0xFF0000FF).value;
  colors[3] = const Color(0xFF00FFFF).value;
  indices[1] = indices[3] = 1;
  indices[2] = indices[5] = 3;
  indices[4] = 2;
  positions[2] = positions[4] = positions[5] = positions[7] = 250.0;

  final Vertices vertices = Vertices.raw(
    VertexMode.triangles,
    positions,
    textureCoordinates: coords,
    colors: colors,
    indices: indices,
  );
  _validateVertices(vertices);
}
void _validateVertices(Vertices vertices) native 'ValidateVertices';

@pragma('vm:entry-point')
void frameCallback(FrameInfo info) {
  print('called back');
}

@pragma('vm:entry-point')
void messageCallback(dynamic data) {
}

@pragma('vm:entry-point')
Future<void> pumpImage() async {
  final Completer<Image> completer = Completer<Image>();
  const int width = 8000;
  const int height = 8000;
  decodeImageFromPixels(
    Uint8List.fromList(List<int>.filled(width * height * 4, 0xFF)),
    width,
    height,
    PixelFormat.rgba8888,
    (Image image) => completer.complete(image),
  );

  final Image image = await completer.future;

  final FrameCallback renderBlank = (Duration duration) {
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawRect(Rect.largest, Paint());
    final Picture picture = recorder.endRecording();

    final SceneBuilder builder = SceneBuilder();
    builder.addPicture(Offset.zero, picture);

    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
    window.onBeginFrame = (Duration duration) => _done();
    window.scheduleFrame();
  };

  final FrameCallback renderImage = (Duration duration) {
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawImage(image, Offset.zero, Paint());
    final Picture picture = recorder.endRecording();

    final SceneBuilder builder = SceneBuilder();
    builder.addPicture(Offset.zero, picture);

    _captureImageAndPicture(image, picture);

    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
    picture.dispose();
    image.dispose();
    window.onBeginFrame = renderBlank;
    window.scheduleFrame();
  };

  window.onBeginFrame = renderImage;
  window.scheduleFrame();
}
void _captureImageAndPicture(Image image, Picture picture) native 'CaptureImageAndPicture';
Future<void> _done() native 'Done';
