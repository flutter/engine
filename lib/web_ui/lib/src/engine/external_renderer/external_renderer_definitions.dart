// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@JS()
library external_renderer_api;

import 'dart:js_interop';
import 'package:ui/ui.dart' as ui;

import '../dom.dart';

/// Whether `window.flutterWebRenderer` is defined on the current page.
final bool isExternalFlutterWebRendererDefined =
    _windowFlutterWebRenderer != null;

/// The external renderer supplied by the application developer.
///
/// This variable is only valid if [isExternalFlutterWebRendererDefined] is
/// true. Otherwise, accessing this variable will throw a [UnsupportedError].
final ExternalRendererDef externalFlutterWebRenderer = () {
  final windowFlutterWebRenderer = _windowFlutterWebRenderer;

  if (windowFlutterWebRenderer == null) {
    throw UnsupportedError('window.flutterWebRenderer not defined');
  }

  return windowFlutterWebRenderer;
}();

@JS('window.flutterWebRenderer')
external ExternalRendererDef? get _windowFlutterWebRenderer;

extension type ExternalRendererDef._(JSObject _) implements JSObject {
  external void clearFragmentProgramCache();

  external ExternalPathDef combinePaths(
    ExternalPathOperationDef index,
    ExternalPathDef path1,
    ExternalPathDef path2,
  );

  external ExternalImageFilterDef composeImageFilters(
    ExternalImageFilterDef outer,
    ExternalImageFilterDef inner,
  );

  external ExternalPathDef copyPath(ExternalPathDef src);

  external ExternalImageFilterDef createBlurImageFilter(
    JSNumber sigmaX,
    JSNumber sigmaY,
    ExternalTileModeDef tileMode,
  );

  external ExternalCanvasDef createCanvas(
    ExternalPictureRecorderDef recorder,
    ExternalRectDef? cullRect,
  );

  external ExternalGradientDef createConicalGradient(
    ExternalOffsetDef focal,
    JSNumber focalRadius,
    ExternalOffsetDef center,
    JSNumber radius,
    JSArray<ExternalColorDef> colors,
    JSArray<JSNumber>? colorStops,
    ExternalTileModeDef tileMode,
    JSFloat32Array? matrix,
  );

  external ExternalImageFilterDef createDilateImageFilter(
    JSNumber radiusX,
    JSNumber radiusY,
  );

  external ExternalImageFilterDef createErodeImageFilter(
    JSNumber radiusX,
    JSNumber radiusY,
  );

  external JSPromise<ExternalFragmentProgramDef> createFragmentProgram(JSString assetKey);

  external JSPromise<ExternalImageDef> createImageFromImageBitmap(DomImageBitmap imageSource);

  external JSPromise<ExternalImageDef> createImageFromTextureSource(
    JSAny object,
    JSNumber width,
    JSNumber height,
    JSBoolean transferOwnership,
  );
}

extension type ExternalRectDef._(JSObject _) implements JSObject {
  external ExternalRectDef.fromLTRB({
    required JSNumber left,
    required JSNumber top,
    required JSNumber right,
    required JSNumber bottom,
  });

  external JSNumber get left;
  external JSNumber get top;
  external JSNumber get right;
  external JSNumber get bottom;
}

extension ExternalRectDefExtension on ui.Rect {
  ExternalRectDef get toJS {
    return ExternalRectDef.fromLTRB(
      left: left.toJS,
      top: top.toJS,
      right: right.toJS,
      bottom: bottom.toJS,
    );
  }
}

extension type ExternalOffsetDef._(JSObject _) implements JSObject {
  external ExternalOffsetDef.fromLTRB({
    required JSNumber dx,
    required JSNumber dy,
  });

  external JSNumber get dx;
  external JSNumber get dy;
}

extension ExternalOffsetDefExtension on ui.Offset {
  ExternalOffsetDef get toJS {
    return ExternalOffsetDef.fromLTRB(
      dx: dx.toJS,
      dy: dy.toJS,
    );
  }
}

extension type ExternalColorDef._(JSObject _) implements JSObject {
  external ExternalColorDef.fromLTRB({
    required JSNumber a,
    required JSNumber r,
    required JSNumber g,
    required JSNumber b,
    required ExternalColorSpaceDef colorSpace,
  });

  external JSNumber get a;
  external JSNumber get r;
  external JSNumber get g;
  external JSNumber get b;
  external ExternalColorSpaceDef get colorSpace;
}

extension ExternalColorDefExtension on ui.Color {
  ExternalColorDef get toJS {
    return ExternalColorDef.fromLTRB(
      a: a.toJS,
      r: r.toJS,
      g: g.toJS,
      b: b.toJS,
      colorSpace: colorSpace.toJS,
    );
  }
}

extension ExternalColorDefList on List<ui.Color> {
  JSArray<ExternalColorDef> get toJS {
    final result = JSArray<ExternalColorDef>();
    for (final color in this) {
      result.add(color.toJS);
    }
    return result;
  }
}

extension type ExternalPathDef._(JSObject _) implements JSObject {}

extension type ExternalImageFilterDef._(JSObject _) implements JSObject {}

extension type ExternalCanvasDef._(JSObject _) implements JSObject {}

extension type ExternalGradientDef._(JSObject _) implements JSObject {}

extension type ExternalFragmentProgramDef._(JSObject _) implements JSObject {}

extension type ExternalImageDef._(JSObject _) implements JSObject {}

extension type ExternalImageShaderDef._(JSObject _) implements JSObject {}

extension type ExternalLineMetricsDef._(JSObject _) implements JSObject {}

extension type ExternalPaintDef._(JSObject _) implements JSObject {}

extension type ExternalParagraphBuilderDef._(JSObject _) implements JSObject {}

extension type ExternalParagraphDef._(JSObject _) implements JSObject {}

extension type ExternalParagraphStyleDef._(JSObject _) implements JSObject {}

extension type ExternalPictureRecorderDef._(JSObject _) implements JSObject {}

extension type ExternalSceneBuilderDef._(JSObject _) implements JSObject {}

extension type ExternalBackdropFilterEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalClipPathEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalClipRRectEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalClipRectEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalColorFilterEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalImageFilterEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalOffsetEngineLayerDef._(JSObject _) implements JSObject {
  external void dispose();
}

extension type ExternalOpacityEngineLayerDef._(JSObject _) implements JSObject {
  external void dispose();
}

extension type ExternalShaderMaskEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

extension type ExternalTransformEngineLayerDef._(JSObject _)
    implements JSObject {
  external void dispose();
}

// Enums

extension type ExternalTileModeDef._(JSNumber _) {}
extension ExternalTileModeDefExtension on ui.TileMode {
  ExternalTileModeDef get toJS {
    return ExternalTileModeDef._(index.toJS);
  }
}

extension type ExternalColorSpaceDef._(JSNumber _) {}
extension ExternalColorSpaceDefExtension on ui.ColorSpace {
  ExternalColorSpaceDef get toJS {
    return ExternalColorSpaceDef._(index.toJS);
  }
}

extension type ExternalPathOperationDef._(JSNumber _) {}
extension ExternalPathOperationExtension on ui.PathOperation {
  ExternalPathOperationDef get toJS {
    return ExternalPathOperationDef._(index.toJS);
  }
}

// Generic type conversions

extension DoubleListToJSArrayList on List<double> {
  JSArray<JSNumber> get toJSNumberArray {
    final result = JSArray<JSNumber>();
    for (final element in this) {
      result.add(element.toJS);
    }
    return result;
  }
}
