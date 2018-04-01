// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ui';
import 'dart:typed_data';
import 'dart:io';

import 'package:test/test.dart';
import 'package:path/path.dart' as path;

void main() {

  Image createSquareTestImage() {
    PictureRecorder recorder = new PictureRecorder();
    Canvas canvas = new Canvas(recorder, new Rect.fromLTWH(0.0, 0.0, 10.0, 10.0));

    var black = new Paint()
      ..strokeWidth = 1.0
      ..color = const Color.fromRGBO(0, 0, 0, 1.0);
    var green = new Paint()
      ..strokeWidth = 1.0
      ..color = const Color.fromRGBO(0, 255, 0, 1.0);

    canvas.drawRect(new Rect.fromLTWH(0.0, 0.0, 10.0, 10.0), black);
    canvas.drawRect(new Rect.fromLTWH(2.0, 2.0, 6.0, 6.0), green);
    return recorder.endRecording().toImage(10, 10);
  }

  Uint8List readFile(fileName) {
    final file = new File(path.join('flutter', 'testing', 'resources', fileName));
    return file.readAsBytesSync();
  }

  final Image testImage = createSquareTestImage();

  test('Encode with default arguments', () async {
    Uint8List data = await testImage.toByteData();
    Uint8List expected = readFile('square-80.jpg');
    expect(data, expected);
  });

  test('Encode JPEG', () async {
    Uint8List data = await testImage.toByteData(format: EncodeFormat.JPEG, quality:80);
    Uint8List expected = readFile('square-80.jpg');
    expect(data, expected);
  });

  test('Encode PNG', () async {
    Uint8List data = await testImage.toByteData(format: EncodeFormat.PNG, quality: 80);
    Uint8List expected = readFile('square-80.png');
    expect(data, expected);
  });

  test('Encode WEBP', () async {
    Uint8List data = await testImage.toByteData(format: EncodeFormat.WEBP, quality: 80);
    Uint8List expected = readFile('square-80.webp');
    expect(data, expected);
  });
}