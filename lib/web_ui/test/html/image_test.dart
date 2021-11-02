// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' hide TextStyle;

import './testimage.dart';

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

Future<void> testMain() async {
  test('Draws image with dstATop color filter', () async {
    const List<int> testImage = <int>[0xFA000000, 0x00FA0030, 0x0000FA60, 0x000000FF];
    final Uint8List sourcePixels = Uint8List.sublistView(Uint32List.fromList(testImage));
    final ImageDescriptor encoded = ImageDescriptor.raw(
      await ImmutableBuffer.fromUint8List(sourcePixels),
      width: 2,
      height: 2,
      pixelFormat: PixelFormat.rgba8888,
    );
    final Image decoded = (await (await encoded.instantiateCodec()).getNextFrame()).image;
    final Uint8List actualPixels  = Uint8List.sublistView(
        (await decoded.toByteData(format: ImageByteFormat.rawStraightRgba))!);
    expect(actualPixels, hasLength(sourcePixels.length));
    expect(actualPixels, predicate(deepEqualList(sourcePixels), sourcePixels.toString()));
  });
}
