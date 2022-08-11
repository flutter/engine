// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/test.dart';
import 'package:ui/ui.dart';

void main() {
  test('Image constructor invokes onCreate once', () async {
    int onCreateInvokedCount = 0;
    Image? createdImage;
    Image.onCreate = (Image image) {
      onCreateInvokedCount++;
      createdImage = image;
    };

    final Image image1 = await _createImage();

    expect(onCreateInvokedCount, 1);
    expect(createdImage, image1);

    final Image image2 = await _createImage();

    expect(onCreateInvokedCount, 2);
    expect(createdImage, image2);
    Image.onCreate = null;
  });

  test('dispose() invokes onDispose once', () async {
    int onDisposeInvokedCount = 0;
    Image? disposedImage;
    Image.onDispose = (Image image) {
      onDisposeInvokedCount++;
      disposedImage = image;
    };

    final Image image1 = await _createImage()..dispose();

    expect(onDisposeInvokedCount, 1);
    expect(disposedImage, image1);

    final Image image2 = await _createImage()..dispose();

    expect(onDisposeInvokedCount, 2);
    expect(disposedImage, image2);

    Image.onDispose = null;
  });
}


Future<Image> _createImage() async => _createPicture().toImage(10, 10);

Picture _createPicture() {
  final PictureRecorder recorder = PictureRecorder();
  final Canvas canvas = Canvas(recorder);
  const Rect rect = Rect.fromLTWH(0.0, 0.0, 100.0, 100.0);
  canvas.clipRect(rect);
  return recorder.endRecording();
}
