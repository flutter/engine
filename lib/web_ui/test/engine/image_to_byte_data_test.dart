// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:typed_data';

import 'package:ui/ui.dart';
import 'package:ui/src/engine.dart';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() async {
  setUp(() async {
    await webOnlyInitializePlatform();
    webOnlyFontCollection.debugRegisterTestFonts();
    await webOnlyFontCollection.ensureFontsLoaded();
  });

  test('Picture.toImage().toByteData()', () async {
    final EnginePictureRecorder recorder = PictureRecorder();
    final RecordingCanvas canvas =
        recorder.beginRecording(Rect.fromLTRB(0, 0, 2, 2));
    canvas.drawColor(Color(0xFFCCDD00), BlendMode.srcOver);
    final Picture testPicture = recorder.endRecording();
    final Image testImage = await testPicture.toImage(2, 2);
    final ByteData bytes =
        await testImage.toByteData(format: ImageByteFormat.rawRgba);
    expect(
      bytes.buffer.asUint32List(),
      <int>[0xFF00DDCC, 0xFF00DDCC, 0xFF00DDCC, 0xFF00DDCC],
    );

    final ByteData pngBytes =
        await testImage.toByteData(format: ImageByteFormat.png);
    expect(
      pngBytes.buffer.asUint32List(),
      <int>[
        0x474E5089,
        0xA1A0A0D,
        0xD000000,
        0x52444849,
        0x5000000,
        0x5000000,
        0x608,
        0x266F8D00,
        0xE5,
        0x41444913,
        0x63571854,
        0xE197733C,
        0x601A033F,
        0x2081A4,
        0x4E0DFF25,
        0xED54E7EE,
        0x0,
        0x444E4549,
        0x826042AE
      ],
    );
  });
}
