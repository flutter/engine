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

  test('basic image descriptor - encoded - greyscale', () async {
    final Uint8List bytes = await readFile('2x2.png');
    final ImmutableBuffer buffer = await ImmutableBuffer.fromUint8List(bytes);
    final ImageDescriptor descriptor = await ImageDescriptor.encoded(buffer);

    expect(descriptor.width, 2);
    expect(descriptor.height, 2);
    expect(descriptor.bytesPerPixel, 1);

    final Codec codec = await descriptor.instantiateCodec();
    expect(codec.frameCount, 1);
  });

  test('basic image descriptor - encoded - square', () async {
    final Uint8List bytes = await readFile('square.png');
    final ImmutableBuffer buffer = await ImmutableBuffer.fromUint8List(bytes);
    final ImageDescriptor descriptor = await ImageDescriptor.encoded(buffer);

    expect(descriptor.width, 10);
    expect(descriptor.height, 10);
    expect(descriptor.bytesPerPixel, 4);

    final Codec codec = await descriptor.instantiateCodec();
    expect(codec.frameCount, 1);
  });

  test('basic image descriptor - encoded - animated', () async {
    final Uint8List bytes = await _getSkiaResource('test640x479.gif').readAsBytes();
    final ImmutableBuffer buffer = await ImmutableBuffer.fromUint8List(bytes);
    final ImageDescriptor descriptor = await ImageDescriptor.encoded(buffer);

    expect(descriptor.width, 640);
    expect(descriptor.height, 479);
    expect(descriptor.bytesPerPixel, 4);

    final Codec codec = await descriptor.instantiateCodec();
    expect(codec.frameCount, 4);
    expect(codec.repetitionCount, -1);
  });

  test('basic image descriptor - raw', () async {
    final Uint8List bytes = Uint8List.fromList(List<int>.filled(16, 0xFFABCDEF));
    final ImmutableBuffer buffer = await ImmutableBuffer.fromUint8List(bytes);
    final ImageDescriptor descriptor = ImageDescriptor.raw(
      buffer,
      width: 4,
      height: 4,
      rowBytes: 4 * 4,
      pixelFormat: PixelFormat.rgba8888,
    );

    expect(descriptor.width, 4);
    expect(descriptor.height, 4);
    expect(descriptor.bytesPerPixel, 4);

    final Codec codec = await descriptor.instantiateCodec();
    expect(codec.frameCount, 1);
  });

  test('HEIC image', () async {
    final Uint8List bytes = await readFile('grill_chicken.heic');
    final ImmutableBuffer buffer = await ImmutableBuffer.fromUint8List(bytes);
    final ImageDescriptor descriptor = await ImageDescriptor.encoded(buffer);

    expect(descriptor.width, 300);
    expect(descriptor.height, 400);
    expect(descriptor.bytesPerPixel, 4);

    final Codec codec = await descriptor.instantiateCodec();
    expect(codec.frameCount, 1);
  }, skip: !(Platform.isAndroid || Platform.isIOS || Platform.isMacOS || Platform.isWindows));

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
