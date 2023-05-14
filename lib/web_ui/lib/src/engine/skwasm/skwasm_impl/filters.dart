// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

class SkwasmImageFilter implements ui.ImageFilter {
  SkwasmImageFilter._(this.handle);

  factory SkwasmImageFilter.blur({
    double sigmaX = 0.0,
    double sigmaY = 0.0,
    ui.TileMode tileMode = ui.TileMode.clamp,
  }) => SkwasmImageFilter._(imageFilterCreateBlur(sigmaX, sigmaY, tileMode.index));

  factory SkwasmImageFilter.dilate({
    double radiusX = 0.0,
    double radiusY = 0.0,
  }) => SkwasmImageFilter._(imageFilterCreateDilate(radiusX, radiusY));

  factory SkwasmImageFilter.erode({
    double radiusX = 0.0,
    double radiusY = 0.0,
  }) => SkwasmImageFilter._(imageFilterCreateErode(radiusX, radiusY));

  factory SkwasmImageFilter.matrix(
    Float64List matrix4, {
    ui.FilterQuality filterQuality = ui.FilterQuality.low
  }) => withStackScope((StackScope scope) => SkwasmImageFilter._(imageFilterCreateMatrix(
    scope.convertMatrix4toSkMatrix(matrix4),
    filterQuality.index
  )));

  factory SkwasmImageFilter.compose(
    SkwasmImageFilter outer,
    SkwasmImageFilter inner,
  ) => SkwasmImageFilter._(imageFilterCompose(outer.handle, inner.handle));

  void dispose() {
    if (!_isDisposed) {
      imageFilterDispose(handle);
      _isDisposed = true;
    }
  }

  final ImageFilterHandle handle;
  bool _isDisposed = false;
}

class SkwasmColorFilter {
  SkwasmColorFilter._(this.handle);

  factory SkwasmColorFilter.fromEngineColorFilter(EngineColorFilter colorFilter) =>
    switch (colorFilter.type) {        
      ColorFilterType.mode => SkwasmColorFilter._(colorFilterCreateMode(
        colorFilter.color!.value,
        colorFilter.blendMode!.index,
      )),
      ColorFilterType.linearToSrgbGamma => linearToSrgbGamma,
      ColorFilterType.srgbToLinearGamma => srgbToLinearGamma,
      ColorFilterType.matrix => withStackScope((StackScope scope) {
        final Pointer<Float> nativeMatrix = scope.convertDoublesToNative(colorFilter.matrix!);
        return SkwasmColorFilter._(colorFilterCreateMatrix(nativeMatrix));
      }),
    };

  static SkwasmColorFilter linearToSrgbGamma = SkwasmColorFilter._(colorFilterCreateLinearToSRGBGamma());
  static SkwasmColorFilter srgbToLinearGamma = SkwasmColorFilter._(colorFilterCreateSRGBToLinearGamma());

  void dispose() {
    if (!_isDisposed) {
      colorFilterDispose(handle)
      _isDisposed = true;
    }
  }

  final ColorFilterHandle handle;
  bool _isDisposed = false;
}

class SkwasmMaskFilter {
  SkwasmMaskFilter._(this.handle);

  factory SkwasmMaskFilter.fromUiMaskFilter(ui.MaskFilter maskFilter) =>
    SkwasmMaskFilter._(maskFilterCreateBlur(
      maskFilter.webOnlyBlurStyle.index,
      maskFilter.webOnlySigma
    ));

  final MaskFilterHandle handle;
  bool _isDisposed = false;

  void dispose() {
    if (!_isDisposed) {
      maskFilterDispose(handle);
      _isDisposed = true;
    }
  }
}
