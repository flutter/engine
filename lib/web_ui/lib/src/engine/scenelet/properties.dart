// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:meta/meta.dart';
import 'package:ui/ui.dart' as ui;

import 'scenelet.dart';

/// Base class for a family of classes providing layer properties.
///
/// At a minimum all layers have the [id] property, which is a unique layer
/// identifier. When the framework constructs a new layer but provides the same
/// [id] value, the engine should treat it as an update to an existing layer.
@immutable
abstract class LayerProperties {
  LayerProperties();

  static int _layerIdCounter = 0;

  /// A unique identifier for the corresponding scene layer used for efficient
  /// layer tree diffing.
  ///
  /// This identifier may be shared by multiple scenelet layers. This is because
  /// one scene layer may be split up into multiple scenelet layers.
  // TODO(yjbanov): make this an inline class when available. Raw int as a type
  //                is meaningless. All we need to be able to do with IDs is ==
  //                compare them (general math makes no sense here).
  final int id = _layerIdCounter++;

  /// Pushes these layer properties onto a scenelet builder layer stack.
  ///
  /// Concrete implementations are expected to call the concrete "push" method
  /// on [SceneletRenderTreeBuilder].
  SceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId);
}

class BackdropFilterLayerProperties extends LayerProperties {
  BackdropFilterLayerProperties({
    required this.filter,
    required this.blendMode,
  });

  final ui.ImageFilter filter;
  final ui.BlendMode blendMode;

  @override
  BackdropFilterSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushBackdropFilter(this, oldId: oldId);
  }
}

class ClipPathLayerProperties extends LayerProperties {
  ClipPathLayerProperties({
    required this.path,
    required this.clipBehavior,
  });

  final ui.Path path;
  final ui.Clip clipBehavior;

  @override
  ClipPathSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushClipPath(this, oldId: oldId);
  }
}

class ClipRRectLayerProperties extends LayerProperties {
  ClipRRectLayerProperties({
    required this.rrect,
    required this.clipBehavior,
  });

  final ui.RRect rrect;
  final ui.Clip clipBehavior;

  @override
  ClipRRectSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushClipRRect(this, oldId: oldId);
  }
}

class ClipRectLayerProperties extends LayerProperties {
  ClipRectLayerProperties({
    required this.rect,
    required this.clipBehavior,
  });

  final ui.Rect rect;
  final ui.Clip clipBehavior;

  @override
  ClipRectSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushClipRect(this, oldId: oldId);
  }
}

class ColorFilterLayerProperties extends LayerProperties {
  ColorFilterLayerProperties({
    required this.filter,
  });

  final ui.ColorFilter filter;

  @override
  ColorFilterSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushColorFilter(this, oldId: oldId);
  }
}

class ImageFilterLayerProperties extends LayerProperties {
  ImageFilterLayerProperties({
    required this.filter,
    required this.offset,
  });

  final ui.ImageFilter filter;
  final ui.Offset offset;

  @override
  ImageFilterSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushImageFilter(this, oldId: oldId);
  }
}

class OffsetLayerProperties extends LayerProperties {
  OffsetLayerProperties({
    required this.dx,
    required this.dy,
  });

  final double dx;
  final double dy;

  @override
  OffsetSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushOffset(this, oldId: oldId);
  }
}

class OpacityLayerProperties extends LayerProperties {
  OpacityLayerProperties({
    required this.alpha,
    required this.offset,
  });

  final int alpha;
  final ui.Offset offset;

  @override
  OpacitySceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushOpacity(this, oldId: oldId);
  }
}

class PhysicalShapeLayerProperties extends LayerProperties {
  PhysicalShapeLayerProperties({
    required this.path,
    required this.elevation,
    required this.color,
    required this.shadowColor,
    required this.clipBehavior,
  });

  final ui.Path path;
  final double elevation;
  final ui.Color color;
  final ui.Color? shadowColor;
  final ui.Clip clipBehavior;

  @override
  PhysicalShapeSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushPhysicalShape(this, oldId: oldId);
  }
}

class ShaderMaskLayerProperties extends LayerProperties {
  ShaderMaskLayerProperties({
    required this.shader,
    required this.maskRect,
    required this.blendMode,
    required this.filterQuality,
  });

  final ui.Shader shader;
  final ui.Rect maskRect;
  final ui.BlendMode blendMode;
  final ui.FilterQuality filterQuality;

  @override
  ShaderMaskSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushShaderMask(this, oldId: oldId);
  }
}

class TransformLayerProperties extends LayerProperties {
  TransformLayerProperties({
    required this.matrix4,
  });

  final Float64List matrix4;

  @override
  TransformSceneletLayer pushToScenelet(SceneletRenderTreeBuilder treeBuilder, int? oldId) {
    return treeBuilder.pushTransform(this, oldId: oldId);
  }
}
