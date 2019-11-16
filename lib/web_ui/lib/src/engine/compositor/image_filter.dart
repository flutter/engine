// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// The CanvasKit implementation of [ui.ImageFilter].
///
/// Currently only supports `blur`.
class SkImageFilter implements ui.ImageFilter {
  js.JsObject skImageFilter;

  SkImageFilter.blur({this.sigmaX = 0.0, this.sigmaY = 0.0}) {
    skImageFilter = canvasKit['SkImageFilter'].callMethod(
      'MakeBlur',
      <dynamic>[
        sigmaX,
        sigmaY,
        canvasKit['TileMode']['Clamp'],
        null,
      ],
    );
  }

  final double sigmaX;
  final double sigmaY;

  @override
  bool operator ==(dynamic other) {
    if (other is! SkImageFilter) {
      return false;
    }
    final SkImageFilter typedOther = other;
    return sigmaX == typedOther.sigmaX && sigmaY == typedOther.sigmaY;
  }

  @override
  int get hashCode => ui.hashValues(sigmaX, sigmaY);

  @override
  String toString() {
    return 'ImageFilter.blur($sigmaX, $sigmaY)';
  }
}
