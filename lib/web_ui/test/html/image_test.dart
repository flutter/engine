// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' hide TextStyle;

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

typedef _ListPredicate<T> = bool Function(List<T>);
_ListPredicate<T> deepEqualList<T>(List<T> a) {
  return (List<T> b) {
    if (a.length != b.length)
      return false;
    for (int i = 0; i < a.length; i += 1) {
      if (a[i] != b[i])
        return false;
    }
    return true;
  };
}

Matcher listEqual<T>(List<T> source) {
  return predicate(
    (List<T> target) {
      if (source.length != target.length)
        return false;
      for (int i = 0; i < source.length; i += 1) {
        if (source[i] != target[i])
          return false;
      }
      return true;
    },
    source.toString(),
  );
}

Future<void> testMain() async {
  test('Correctly encodes an opaque image', () async {
    // A 2x2 testing image without transparency.
    // Pixel order: Left to right, then top to bottom.
    // Byte order: 0xAABBGGRR (because uint8 is placed in little endian.)
    final Uint8List sourceImage = Uint8List.sublistView(Uint32List.fromList(
      <int>[0xFF0201FF, 0xFF05FE04, 0xFFFD0807, 0x000C0B0A],
    ));
    final ImageDescriptor descriptor = ImageDescriptor.raw(
      await ImmutableBuffer.fromUint8List(sourceImage),
      width: 2,
      height: 2,
      pixelFormat: PixelFormat.rgba8888,
    );
    final Image encoded = (await (await descriptor.instantiateCodec()).getNextFrame()).image;
    final Uint8List actualPixels  = Uint8List.sublistView(
        (await encoded.toByteData(format: ImageByteFormat.rawStraightRgba))!);
    final Uint8List targetImage = Uint8List.sublistView(Uint32List.fromList(
      <int>[0xFF0201FF, 0xFF05FE04, 0xFFFD0807, 0x00000000],
    ));
    expect(actualPixels, listEqual(targetImage));
  });

  test('Correctly encodes a transparent image', () async {
    // A 2x2 testing image with transparency.
    // Pixel order: Left to right, then top to bottom.
    // Byte order: 0xAABBGGRR (because uint8 is placed in little endian.)
    final Uint8List sourceImage = Uint8List.sublistView(Uint32List.fromList(
      <int>[0x030201FF, 0x0605FE04, 0x09FD0807, 0xFC0C0B0A],
    ));
    final ImageDescriptor descriptor = ImageDescriptor.raw(
      await ImmutableBuffer.fromUint8List(sourceImage),
      width: 2,
      height: 2,
      pixelFormat: PixelFormat.rgba8888,
    );
    final Image encoded = (await (await descriptor.instantiateCodec()).getNextFrame()).image;
    final Uint8List actualPixels  = Uint8List.sublistView(
        (await encoded.toByteData(format: ImageByteFormat.rawStraightRgba))!);
    // TODO(dkwingsmt): Known bug: The `targetImage` is slight differnt from
    // `sourceImage` due to unknown reasons (possibly because how
    // canvas.drawImage blends transparent pixels). In an ideal world we should
    // use `sourceImage` here.
    // https://github.com/flutter/flutter/issues/92958
    final Uint8List targetImage = Uint8List.sublistView(Uint32List.fromList(
      <int>[0x030000FF, 0x0600FF00, 0x09FF0000, 0xFC0C0B0A],
    ));
    expect(actualPixels, listEqual(targetImage));
  });
}
