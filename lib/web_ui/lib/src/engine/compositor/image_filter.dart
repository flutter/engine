// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

class SkImageFilter implements ui.ImageFilter {
  js.JsObject skImageFilter;

  SkImageFilter.blur({double sigmaX = 0.0, double sigmaY = 0.0}) {
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

  double get sigmaX => throw UnimplementedError(
      'CanvasKit backend doesn\'t support ImageFilter.sigmaX');

  double get sigmaY => throw UnimplementedError(
      'CanvasKit backend doesn\'t support ImageFilter.sigmaY');
}
