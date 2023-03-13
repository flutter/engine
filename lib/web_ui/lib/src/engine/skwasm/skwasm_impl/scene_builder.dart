// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

mixin PictureLayer implements ui.EngineLayer {
  ui.Picture? picture;

  void dispose() {
    picture?.dispose();
  }
}

abstract class LayerOperation {
  void pre(ui.Canvas canvas);
  void post(ui.Canvas canvas);
}

class LayerBuilder {
  LayerBuilder._(
    this.recorder,
    this.canvas,
    this.parent,
    this.layer,
    this.operation
  );

  factory LayerBuilder() {
    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Canvas canvas = ui.Canvas(recorder, ui.Rect.largest);
    return LayerBuilder._(recorder, canvas);
  }

  final LayerBuilder? parent;
  final ui.PictureRecorder recorder;
  final ui.Canvas canvas;
  final PictureLayer? layer;
  final LayerOperation? operation;

  void prepare() {
    operation.pre(canvas);
  }

  ui.Picture build() {
    operation.post(canvas);
    final ui.Picture picture = recorder.endRecording();
    layer?.picture = picture;
    return picture;
  }

  void addPicture(
    ui.Offset offset,
    ui.Picture picture, {
    bool isComplexHint = false, 
    bool willChangeHint = false
  }) {
    if (offset != ui.Offset.zero) {
      canvas.save();
      canvas.translate(offset.dx, offset.dy);
      canvas.drawPicture(picture);
      canvas.restore();
    } else {
      canvas.drawPicture(picture);
    }
  }
}

class SkwasmSceneBuilder implements ui.SceneBuilder {
  LayerBuilder currentBuilder = LayerBuilder();

  @override
  void addPerformanceOverlay(int enabledOptions, ui.Rect bounds) {
    // We don't plan to implement this on the web.
    throw UnimplementedError();
  }

  @override
  void addPicture(
    ui.Offset offset,
    ui.Picture picture, {
    bool isComplexHint = false, 
    bool willChangeHint = false
  }) {
    layerBuilder.addPicture(
      offset,
      picture,
      isComplexHint:
      isComplexHint,
      willChangeHint: willChangeHint
    );
  }

  @override
  void addPlatformView(
    int viewId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0
  }) {
    throw UnimplementedError("Platform view not implemented with skwasm renderer.");
  }

  @override
  void addRetained(ui.EngineLayer retainedLayer) {
    // TODO(jacksongardner): implement addRetained
  }

  @override
  void addTexture(
    int textureId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0,
    bool freeze = false,
    ui.FilterQuality filterQuality = ui.FilterQuality.low
  }) {
    // TODO(jacksongardner): implement addTexture
  }

  @override
  ui.BackdropFilterEngineLayer pushBackdropFilter(
    ui.ImageFilter filter, {
    ui.BlendMode blendMode = ui.BlendMode.srcOver,
    ui.BackdropFilterEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushBackdropFilter
    return ui.BackdropFilterEngineLayer();
  }

  @override
  ui.ClipPathEngineLayer pushClipPath(
    ui.Path path, {
    ui.Clip clipBehavior = ui.Clip.antiAlias,
    ui.ClipPathEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushClipPath
    throw UnimplementedError();
  }

  @override
  ui.ClipRRectEngineLayer pushClipRRect(
    ui.RRect rrect, {
    required ui.Clip clipBehavior,
    ui.ClipRRectEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushClipRRect
    throw UnimplementedError();
  }

  @override
  ui.ClipRectEngineLayer pushClipRect(
    ui.Rect rect, {
    ui.Clip clipBehavior = ui.Clip.antiAlias,
    ui.ClipRectEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushClipRect
    throw UnimplementedError();
  }

  @override
  ui.ColorFilterEngineLayer pushColorFilter(
    ui.ColorFilter filter, {
    ui.ColorFilterEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushColorFilter
    throw UnimplementedError();
  }

  @override
  ui.ImageFilterEngineLayer pushImageFilter(
    ui.ImageFilter filter, {
    ui.Offset offset = ui.Offset.zero,
    ui.ImageFilterEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushImageFilter
    throw UnimplementedError();
  }

  @override
  ui.OffsetEngineLayer pushOffset(
    double dx,
    double dy, {
    ui.OffsetEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushOffset
    throw UnimplementedError();
  }

  @override
  ui.OpacityEngineLayer pushOpacity(int alpha, {
    ui.Offset offset = ui.Offset.zero,
    ui.OpacityEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushOpacity
    throw UnimplementedError();
  }

  @override
  ui.PhysicalShapeEngineLayer pushPhysicalShape({
    required ui.Path path,
    required double elevation,
    required ui.Color color,
    ui.Color? shadowColor,
    ui.Clip clipBehavior = ui.Clip.none,
    ui.PhysicalShapeEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushPhysicalShape
    throw UnimplementedError();
  }

  @override
  ui.ShaderMaskEngineLayer pushShaderMask(
    ui.Shader shader,
    ui.Rect maskRect,
    ui.BlendMode blendMode, {
    ui.ShaderMaskEngineLayer? oldLayer,
    ui.FilterQuality filterQuality = ui.FilterQuality.low
  }) {
    // TODO(jacksongardner): implement pushShaderMask
    throw UnimplementedError();
  }

  @override
  ui.TransformEngineLayer pushTransform(
    Float64List matrix4, {
    ui.TransformEngineLayer? oldLayer
  }) {
    // TODO(jacksongardner): implement pushTransform
    throw UnimplementedError();
  }

  @override
  void setCheckerboardOffscreenLayers(bool checkerboard) {
    // TODO(jacksongardner): implement setCheckerboardOffscreenLayers
  }

  @override
  void setCheckerboardRasterCacheImages(bool checkerboard) {
    // TODO(jacksongardner): implement setCheckerboardRasterCacheImages
  }

  @override
  void setProperties(
    double width,
    double height, 
    double insetTop,
    double insetRight,
    double insetBottom,
    double insetLeft,
    bool focusable
  ) {
    // TODO(jacksongardner): implement setProperties
  }

  @override
  void setRasterizerTracingThreshold(int frameInterval) {
    // TODO(jacksongardner): implement setRasterizerTracingThreshold
  }

  @override
  ui.Scene build() {
    // TODO(jacksongardner): implement build
    throw UnimplementedError();
  }

  @override
  void pop() {
    // TODO(jacksongardner): implement pop
  }
}
