// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('Image constructor invokes onCreate once', () async {
    int onCreateInvokedCount = 0;
    int instanceHashCode = 0;
    Image.onCreate = (Object obj) {
      onCreateInvokedCount++;
      instanceHashCode = identityHashCode(obj);
    };

    final Image image1 = await _createImage();

    expect(onCreateInvokedCount, 1);
    expect(instanceHashCode, identityHashCode(image1));

    final Image image2 = await _createImage();
    
    expect(onCreateInvokedCount, 2);
    expect(instanceHashCode, identityHashCode(image2));
    Image.onCreate = null;
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
