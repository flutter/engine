// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';

import 'package:litetest/litetest.dart';
import 'package:path/path.dart' as path;

void main() {
  tearDown((){
    Image.onDispose = null; 
    Image.onCreate = null; 
  });

  test('Image constructor invokes onCreate once', () async {
    int onCreateInvokedCount = 0;
    int instanceHashCode = 0;
    Image.onCreate = (obj) {
      onCreateInvokedCount++; 
      instanceHashCode = identityHashCode(obj);
    };

    final image1 = _createPicture().toImage();

    expect(onCreateInvokedCount, 1);
    expect(instanceHashCode, identityHashCode(image1));

    final image2 = _createPicture().toImage();
    
    expect(onCreateInvokedCount, 2);
    expect(instanceHashCode, identityHashCode(image2));
  }
}

Picture _createPicture() {
  final recorder = PictureRecorder();
  final canvas = Canvas(recorder);
  var rect = Rect.fromLTWH(0.0, 0.0, 100.0, 100.0);
  canvas.clipRect(rect);
  return recorder.endRecording();
}
