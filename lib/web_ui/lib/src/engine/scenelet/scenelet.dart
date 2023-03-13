// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:meta/meta.dart';
import 'package:ui/ui.dart' as ui;

import 'properties.dart';

/// Builds a [SceneletRenderTree].
class SceneletRenderTreeBuilder {
  /// Creates an empty builder.
  SceneletRenderTreeBuilder() : _containerStack = <ContainerSceneletLayer>[
    RootSceneletLayer(),
  ];

  final List<ContainerSceneletLayer> _containerStack;

  ContainerSceneletLayer get _currentContainer => _containerStack.last;

  int _contentCount = 0;

  int get contentCount => _contentCount;
  bool get hasContent => _contentCount > 0;
  bool get hasNoContent => _contentCount == 0;

  SceneletRenderTree build() {
    return SceneletRenderTree(rootLayer: _containerStack.first as RootSceneletLayer);
  }

  T _pushContainer<T extends ContainerSceneletLayer>(T layer) {
    _currentContainer.appendChild(layer);
    _containerStack.add(layer);
    return layer;
  }

  void pop() {
    _containerStack.removeLast();
  }

  BackdropFilterSceneletLayer pushBackdropFilter(BackdropFilterLayerProperties properties, { int? oldId }) {
    return _pushContainer<BackdropFilterSceneletLayer>(
      BackdropFilterSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  ClipPathSceneletLayer pushClipPath(ClipPathLayerProperties properties, { int? oldId }) {
    return _pushContainer<ClipPathSceneletLayer>(
      ClipPathSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  ClipRRectSceneletLayer pushClipRRect(ClipRRectLayerProperties properties, { int? oldId }) {
    return _pushContainer<ClipRRectSceneletLayer>(
      ClipRRectSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  ClipRectSceneletLayer pushClipRect(ClipRectLayerProperties properties, { int? oldId }) {
    return _pushContainer<ClipRectSceneletLayer>(
      ClipRectSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  ColorFilterSceneletLayer pushColorFilter(ColorFilterLayerProperties properties, { int? oldId }) {
    return _pushContainer<ColorFilterSceneletLayer>(
      ColorFilterSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  ImageFilterSceneletLayer pushImageFilter(ImageFilterLayerProperties properties, { int? oldId }) {
    return _pushContainer<ImageFilterSceneletLayer>(
      ImageFilterSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  OffsetSceneletLayer pushOffset(OffsetLayerProperties properties, { int? oldId }) {
    return _pushContainer<OffsetSceneletLayer>(
      OffsetSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  OpacitySceneletLayer pushOpacity(OpacityLayerProperties properties, { int? oldId }) {
    return _pushContainer<OpacitySceneletLayer>(
      OpacitySceneletLayer(properties: properties, oldId: oldId),
    );
  }

  PhysicalShapeSceneletLayer pushPhysicalShape(PhysicalShapeLayerProperties properties, { int? oldId }) {
    return _pushContainer<PhysicalShapeSceneletLayer>(
      PhysicalShapeSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  ShaderMaskSceneletLayer pushShaderMask(ShaderMaskLayerProperties properties, { int? oldId }) {
    return _pushContainer<ShaderMaskSceneletLayer>(
      ShaderMaskSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  TransformSceneletLayer pushTransform(TransformLayerProperties properties, { int? oldId }) {
    return _pushContainer<TransformSceneletLayer>(
      TransformSceneletLayer(properties: properties, oldId: oldId),
    );
  }

  void addRetained(SceneletLayer retainedLayer) {
    _contentCount += retainedLayer.contentCount;
    _currentContainer.appendChild(retainedLayer);
  }

  void addPicture(ui.Offset offset, ui.Picture picture, {bool isComplexHint = false, bool willChangeHint = false}) {
    _contentCount += 1;
    _currentContainer.appendChild(SceneletPictureLayer(
      offset: offset,
      picture: picture,
      isComplexHint: isComplexHint,
      willChangeHint: willChangeHint,
    ));
  }

  void addTexture(int textureId, {ui.Offset offset = ui.Offset.zero, double width = 0.0, double height = 0.0, bool freeze = false, ui.FilterQuality filterQuality = ui.FilterQuality.low}) {
    _contentCount += 1;
    _currentContainer.appendChild(SceneletTextureLayer(
      textureId: textureId,
      offset: offset,
      width: width,
      height: height,
      freeze: freeze,
      filterQuality: filterQuality,
    ));
  }
}

/// A stack of platform views followed by a [SceneletRenderTree] in paint order.
///
/// Platform views a listed in paint order.
@immutable
class Scenelet {
  Scenelet({
    required this.platformViews,
    required this.renderTree,
  }) : assert(platformViews == null || platformViews.isNotEmpty),
       assert(renderTree == null || renderTree.isNotEmpty);

  final List<SceneletPlatformView>? platformViews;
  final SceneletRenderTree? renderTree;

  void dispose() {
    platformViews?.forEach((SceneletPlatformView view) {
      view.dispose();
    });
    renderTree?.dispose();
  }
}

/// Contains Flutter-native renderable content
///
/// This class plays a similar role to [ui.Scene], except it does not contain
/// platform views.
@immutable
class SceneletRenderTree {
  const SceneletRenderTree({
    required this.rootLayer,
  });

  final RootSceneletLayer rootLayer;

  int get contentCount => rootLayer.contentCount;

  bool get isEmpty => contentCount == 0;

  bool get isNotEmpty => contentCount > 0;

  void dispose() {
    // TODO
  }

  Future<ui.Image> toImage(int width, int height) {
    throw UnimplementedError();
  }

  ui.Image toImageSync(int width, int height) {
    throw UnimplementedError();
  }
}

/// A scenelet containing a platform view.
@immutable
class SceneletPlatformView {
  const SceneletPlatformView({
    required this.containerStack,
    required this.viewId,
    required this.offset,
    required this.width,
    required this.height,
  });

  final List<LayerProperties> containerStack;
  final int viewId;
  final ui.Offset offset;
  final double width;
  final double height;

  void dispose() {
    // TODO(yjbanov)
  }
}

@immutable
abstract class SceneletLayer {
  const SceneletLayer();

  /// The number of atomic renderable pieces of content that this layer has.
  ///
  /// Renderable content includes pictures and textures, each of which
  /// contributes 1 piece of atomic content (i.e. pictures and textures cannot
  /// be split into multiple pieces of content).
  ///
  /// This number includes content of descendant layers.
  ///
  /// This number must be greater than or equal to zero.
  ///
  /// If content count is zero then the layer tree is essentially empty. As an
  /// optimization the rasterizer may ignore this layer and all its descendants,
  /// and it may reorder platform views around this layer to reduce the number
  /// of canvases needed to render the frame.
  int get contentCount;

  void dispose() {
    // TODO(yjbanov);
  }
}

abstract class LeafSceneletLayer extends SceneletLayer {
  const LeafSceneletLayer();

  @override
  int get contentCount => 1;
}

class SceneletPictureLayer extends LeafSceneletLayer {
  const SceneletPictureLayer({
    required this.offset,
    required this.picture,
    required this.isComplexHint,
    required this.willChangeHint,
  });

  final ui.Offset offset;
  final ui.Picture picture;
  final bool isComplexHint;
  final bool willChangeHint;
}

class SceneletTextureLayer extends LeafSceneletLayer {
  const SceneletTextureLayer({
    required this.textureId,
    required this.offset,
    required this.width,
    required this.height,
    required this.freeze,
    required this.filterQuality,
  });

  final int textureId;
  final ui.Offset offset;
  final double width;
  final double height;
  final bool freeze;
  final ui.FilterQuality filterQuality;
}

abstract class ContainerSceneletLayer extends SceneletLayer {
  ContainerSceneletLayer({ required this.oldId });

  final int? oldId;

  List<SceneletLayer> get children => _children;
  final List<SceneletLayer> _children = <SceneletLayer>[];

  @override
  int get contentCount {
    int count = 0;
    for (final SceneletLayer child in _children) {
      count += child.contentCount;
    }
    return count;
  }

  void appendChild(SceneletLayer layer) {
    _children.add(layer);
  }
}

/// An artificial top layer that contains all other layers in a [Scenelet].
///
/// There's no corresponding framework layer for this layer. It is artificially
/// added by [SceneletBuilder] to simplify the code of the builder. For example,
/// the code never needs to check if an ancestor exists, since the root is
/// always there. Nor does it need to deal with the special case of maintaining
/// a list of top-level layers, as those are trivially expressed as normal
/// chilred of the root layer.
class RootSceneletLayer extends ContainerSceneletLayer {
  RootSceneletLayer() : super(oldId: null);
}

class BackdropFilterSceneletLayer extends ContainerSceneletLayer {
  BackdropFilterSceneletLayer({ required this.properties, required super.oldId });

  final BackdropFilterLayerProperties properties;
}

class ClipPathSceneletLayer extends ContainerSceneletLayer {
  ClipPathSceneletLayer({ required this.properties, required super.oldId });

  final ClipPathLayerProperties properties;
}

class ClipRRectSceneletLayer extends ContainerSceneletLayer {
  ClipRRectSceneletLayer({ required this.properties, required super.oldId });

  final ClipRRectLayerProperties properties;
}

class ClipRectSceneletLayer extends ContainerSceneletLayer {
  ClipRectSceneletLayer({ required this.properties, required super.oldId });

  final ClipRectLayerProperties properties;
}

class ColorFilterSceneletLayer extends ContainerSceneletLayer {
  ColorFilterSceneletLayer({ required this.properties, required super.oldId });

  final ColorFilterLayerProperties properties;
}

class ImageFilterSceneletLayer extends ContainerSceneletLayer {
  ImageFilterSceneletLayer({ required this.properties, required super.oldId });

  final ImageFilterLayerProperties properties;
}

class OffsetSceneletLayer extends ContainerSceneletLayer {
  OffsetSceneletLayer({ required this.properties, required super.oldId });

  final OffsetLayerProperties properties;
}

class OpacitySceneletLayer extends ContainerSceneletLayer {
  OpacitySceneletLayer({ required this.properties, required super.oldId });

  final OpacityLayerProperties properties;
}

class PhysicalShapeSceneletLayer extends ContainerSceneletLayer {
  PhysicalShapeSceneletLayer({ required this.properties, required super.oldId });

  final PhysicalShapeLayerProperties properties;
}

class ShaderMaskSceneletLayer extends ContainerSceneletLayer {
  ShaderMaskSceneletLayer({ required this.properties, required super.oldId });

  final ShaderMaskLayerProperties properties;
}

class TransformSceneletLayer extends ContainerSceneletLayer {
  TransformSceneletLayer({ required this.properties, required super.oldId });

  final TransformLayerProperties properties;
}
