// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'package:meta/meta.dart';
import 'package:ui/ui.dart' as ui;

import '../initialization.dart' show platformViewManager;
import 'properties.dart';
import 'scenelet.dart';

/// A scene builder implementation that breaks down the scene into [Scenelet]s.
///
/// The breakdown of content into scenelets allows separating the rendering of
/// Flutter-native content from platform views into two orthogonal aspects.
class SceneletSceneBuilder implements ui.SceneBuilder {
  @override
  void addPerformanceOverlay(int enabledOptions, ui.Rect bounds) {}

  @override
  void setCheckerboardOffscreenLayers(bool checkerboard) {}

  @override
  void setCheckerboardRasterCacheImages(bool checkerboard) {}

  @override
  void setProperties(double width, double height, double insetTop, double insetRight, double insetBottom, double insetLeft, bool focusable) {}

  @override
  void setRasterizerTracingThreshold(int frameInterval) {}

  @override
  SceneletScene build() {
    if (_platformViews.isNotEmpty || _renderTreeBuilder.hasContent) {
      _closeCurrentScenelet();
    }
    return SceneletScene(_scenelets);
  }

  void _closeCurrentScenelet() {
    assert(
      _platformViews.isNotEmpty || _renderTreeBuilder.hasContent,
      'Failed to add a scenelet to a scene. The scenelet would have no '
      'platform views and empty render tree.',
    );
    final SceneletRenderTree? renderTree;

    if (_renderTreeBuilder.hasContent) {
      renderTree = _renderTreeBuilder.build();
      assert(
        renderTree.isNotEmpty,
        'SceneletRenderTreeBuilder.hasContent was true, but SceneletRenderTree '
        'built from it was empty.\n'
        'SceneletRenderTreeBuilder contains ${_renderTreeBuilder.contentCount} pieces of content.\n'
        'SceneletRenderTree contains ${renderTree.contentCount} pieces of content.',
      );
    }

    _scenelets.add(Scenelet(
      platformViews: _platformViews.isNotEmpty ? _platformViews : null,
      renderTree: _renderTreeBuilder.hasContent ? _renderTreeBuilder.build() : null,
    ));
    _renderTreeBuilder = SceneletRenderTreeBuilder();
    _platformViews = <SceneletPlatformView>[];
    _currentSceneletHasRenderContent = false;
  }

  final List<SceneLayer<LayerProperties>> _containerStack = <SceneLayer<LayerProperties>>[];

  /// Completed scenelets built so far.
  final List<Scenelet> _scenelets = <Scenelet>[];

  /// Whether the current scenelet has render content.
  ///
  /// This is flipped to true when the first piece of renderable content, such
  /// as a picture, is added to the scenelet. Only render content and invisible
  /// platform views may be added to the current scenelet after this becomes
  /// true.
  ///
  /// This is flipped to false when the current scenelet is closed and a new one
  /// started.
  bool _currentSceneletHasRenderContent = false;

  /// Builds the render tree for the yet to be built scenelet.
  SceneletRenderTreeBuilder _renderTreeBuilder = SceneletRenderTreeBuilder();

  /// Collects platform views for the yet to be built scenelet.
  List<SceneletPlatformView> _platformViews = <SceneletPlatformView>[];

  T _pushLayer<T extends SceneLayer<LayerProperties>>(T layer) {
    layer.addToScenelet(_renderTreeBuilder);
    _containerStack.add(layer);
    return layer;
  }

  @override
  void pop() {
    _containerStack.removeLast();
    _renderTreeBuilder.pop();
  }

  @override
  BackdropFilterSceneLayer pushBackdropFilter(ui.ImageFilter filter, {ui.BlendMode blendMode = ui.BlendMode.srcOver, ui.BackdropFilterEngineLayer? oldLayer}) {
    oldLayer as BackdropFilterSceneLayer?;
    return _pushLayer<BackdropFilterSceneLayer>(
      BackdropFilterSceneLayer(
        properties: BackdropFilterLayerProperties(
          filter: filter,
          blendMode: blendMode,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  ClipPathSceneLayer pushClipPath(ui.Path path, {ui.Clip clipBehavior = ui.Clip.antiAlias, ui.ClipPathEngineLayer? oldLayer}) {
    oldLayer as ClipPathSceneLayer?;
    return _pushLayer<ClipPathSceneLayer>(
      ClipPathSceneLayer(
        properties: ClipPathLayerProperties(
          path: path,
          clipBehavior: clipBehavior,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  ClipRRectSceneLayer pushClipRRect(ui.RRect rrect, {required ui.Clip clipBehavior, ui.ClipRRectEngineLayer? oldLayer}) {
    oldLayer as ClipRRectSceneLayer?;
    return _pushLayer<ClipRRectSceneLayer>(
      ClipRRectSceneLayer(
        properties: ClipRRectLayerProperties(
          rrect: rrect,
          clipBehavior: clipBehavior,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  ClipRectSceneLayer pushClipRect(ui.Rect rect, {ui.Clip clipBehavior = ui.Clip.antiAlias, ui.ClipRectEngineLayer? oldLayer}) {
    oldLayer as ClipRectSceneLayer?;
    return _pushLayer<ClipRectSceneLayer>(
      ClipRectSceneLayer(
        properties: ClipRectLayerProperties(
          rect: rect,
          clipBehavior: clipBehavior,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  ColorFilterSceneLayer pushColorFilter(ui.ColorFilter filter, {ui.ColorFilterEngineLayer? oldLayer}) {
    oldLayer as ColorFilterSceneLayer?;
    return _pushLayer<ColorFilterSceneLayer>(
      ColorFilterSceneLayer(
        properties: ColorFilterLayerProperties(
          filter: filter,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  ImageFilterSceneLayer pushImageFilter(ui.ImageFilter filter, {ui.Offset offset = ui.Offset.zero, ui.ImageFilterEngineLayer? oldLayer}) {
    oldLayer as ImageFilterSceneLayer?;
    return _pushLayer<ImageFilterSceneLayer>(
      ImageFilterSceneLayer(
        properties: ImageFilterLayerProperties(
          filter: filter,
          offset: offset,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  OffsetSceneLayer pushOffset(double dx, double dy, {ui.OffsetEngineLayer? oldLayer}) {
    oldLayer as OffsetSceneLayer?;
    return _pushLayer<OffsetSceneLayer>(
      OffsetSceneLayer(
        properties: OffsetLayerProperties(
          dx: dx,
          dy: dy,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  OpacitySceneLayer pushOpacity(int alpha, {ui.Offset offset = ui.Offset.zero, ui.OpacityEngineLayer? oldLayer}) {
    oldLayer as OpacitySceneLayer?;
    return _pushLayer<OpacitySceneLayer>(
      OpacitySceneLayer(
        properties: OpacityLayerProperties(
          alpha: alpha,
          offset: offset,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  PhysicalShapeSceneLayer pushPhysicalShape({required ui.Path path, required double elevation, required ui.Color color, ui.Color? shadowColor, ui.Clip clipBehavior = ui.Clip.none, ui.PhysicalShapeEngineLayer? oldLayer}) {
    oldLayer as PhysicalShapeSceneLayer?;
    return _pushLayer<PhysicalShapeSceneLayer>(
      PhysicalShapeSceneLayer(
        properties: PhysicalShapeLayerProperties(
          path: path,
          elevation: elevation,
          color: color,
          shadowColor: shadowColor,
          clipBehavior: clipBehavior,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  ShaderMaskSceneLayer pushShaderMask(ui.Shader shader, ui.Rect maskRect, ui.BlendMode blendMode, {ui.FilterQuality filterQuality = ui.FilterQuality.low, ui.ShaderMaskEngineLayer? oldLayer}) {
    oldLayer as ShaderMaskSceneLayer?;
    return _pushLayer<ShaderMaskSceneLayer>(
      ShaderMaskSceneLayer(
        properties: ShaderMaskLayerProperties(
          shader: shader,
          maskRect: maskRect,
          blendMode: blendMode,
          filterQuality: filterQuality,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  @override
  TransformSceneLayer pushTransform(Float64List matrix4, {ui.TransformEngineLayer? oldLayer}) {
    oldLayer as TransformSceneLayer?;
    return _pushLayer<TransformSceneLayer>(
      TransformSceneLayer(
        properties: TransformLayerProperties(
          matrix4: matrix4,
        ),
        oldId: oldLayer?.properties.id,
      ),
    );
  }

  void _addingRenderContent() {
    _currentSceneletHasRenderContent = true;
  }

  @override
  void addPicture(ui.Offset offset, ui.Picture picture, {bool isComplexHint = false, bool willChangeHint = false}) {
    _addingRenderContent();
    _renderTreeBuilder.addPicture(
      offset,
      picture,
      isComplexHint: isComplexHint,
      willChangeHint: willChangeHint,
    );
  }

  @override
  void addRetained(ui.EngineLayer retainedLayer) {
    _addingRenderContent();

    retainedLayer as SceneLayer<LayerProperties>;

    // The only way a single scene layer can be rendered into multiple scenelet
    // layers is when the scene layer is split up by a platform view inside it.
    assert(
      retainedLayer.sceneletLayers.length == 1,
      'Retained layer cannot contain platform views.',
    );

    _renderTreeBuilder.addRetained(retainedLayer.sceneletLayers.single);
  }

  @override
  void addTexture(int textureId, {ui.Offset offset = ui.Offset.zero, double width = 0.0, double height = 0.0, bool freeze = false, ui.FilterQuality filterQuality = ui.FilterQuality.low}) {
    // TODO: implement addTexture
    _addingRenderContent();
  }

  @override
  void addPlatformView(int viewId, {ui.Offset offset = ui.Offset.zero, double width = 0.0, double height = 0.0}) {
    final List<LayerProperties> propStack = <LayerProperties>[];
    for (final SceneLayer<LayerProperties> sceneLayer in _containerStack) {
      propStack.add(sceneLayer.properties);
      sceneLayer.addToScenelet(_renderTreeBuilder);
    }

    final SceneletPlatformView platformViewScenelet = SceneletPlatformView(
      containerStack: propStack,
      viewId: viewId,
      offset: offset,
      width: width,
      height: height,
    );

    final bool isVisible = platformViewManager.isVisible(viewId);

    // Avoid splitting the scene into scenelets unless it's necessary for
    // correctness. In particular, if all visible platform views of a scene
    // appear behind all Flutter-rendered content, the scene can be rendered
    // using one scenelet (and therefore one canvas). When visible platform
    // views interleave Flutter-rendered content, use as few scenelets as
    // possible without sacrificing visual and interactive correctness.
    if (isVisible && _currentSceneletHasRenderContent) {
      // Adding a visible platform view after rendering some rendered content.
      // The platform view cannot be added to the existing scenelet, because it
      // will appear behind rendered content. Therefore the current scenelet
      // must be closed and added to the scenelet list, and a new one must be
      // started.
      //
      // This does not matter in other scenarios:
      //
      //  * Adding an invisible platform view: invisible platform views are not
      //    affected by paint order relative to rendered content. However, it is
      //    affected by insertion order relative to other platform views. So
      //    there's no need to close the scenelet, but the platform view must go
      //    after the last added platform view, if any, which applies to all
      //    platform views anyway.
      //  * No rendered content was added prior to this platform view: there's
      //    no need to start a new scenelet. Simply keep appending platform
      //    views in paint order.
      _closeCurrentScenelet();
    }

    assert(
      !(isVisible && _currentSceneletHasRenderContent),
      'This is a Flutter Web Engine bug. Previous logic must have ensured that '
      'a visible platform view that follows render content in paint order is '
      'not added to the same scenelet with the render content.'
    );
    _platformViews.add(platformViewScenelet);
  }
}

/// A [ui.Scene] implementation that's made of [Scenelet]s.
class SceneletScene implements ui.Scene {
  const SceneletScene(this.scenelets);

  /// Scenelets comprising this scene.
  ///
  /// The order of scenelets in this list matters. Scenelets at the beginning of
  /// of the list appear before scenelets later in the list visually and in
  /// terms of hit-test order.
  final List<Scenelet> scenelets;

  @override
  void dispose() {
    for (final Scenelet scenelet in scenelets) {
      scenelet.dispose();
    }
  }

  @override
  Future<ui.Image> toImage(int width, int height) {
    throw UnimplementedError();
  }

  @override
  ui.Image toImageSync(int width, int height) {
    throw UnimplementedError();
  }
}

/// A unique value assigned to to an [EngineLayer].
@immutable
abstract class SceneLayer<T extends LayerProperties> implements ui.EngineLayer {
  SceneLayer({ required this.properties, required this.oldId });

  final T properties;
  final int? oldId;
  final List<SceneletLayer> sceneletLayers = <SceneletLayer>[];

  void addToScenelet(SceneletRenderTreeBuilder renderSceneletBuilder) {
    sceneletLayers.add(properties.pushToScenelet(renderSceneletBuilder, oldId));
  }

  @override
  void dispose() {
    for (final SceneletLayer sceneletLayer in sceneletLayers) {
      sceneletLayer.dispose();
    }
  }
}

class BackdropFilterSceneLayer extends SceneLayer<BackdropFilterLayerProperties> implements ui.BackdropFilterEngineLayer {
  BackdropFilterSceneLayer({ required super.properties, required super.oldId});
}

class ClipPathSceneLayer extends SceneLayer<ClipPathLayerProperties> implements ui.ClipPathEngineLayer {
  ClipPathSceneLayer({ required super.properties, required super.oldId});
}

class ClipRRectSceneLayer extends SceneLayer<ClipRRectLayerProperties> implements ui.ClipRRectEngineLayer {
  ClipRRectSceneLayer({ required super.properties, required super.oldId});
}

class ClipRectSceneLayer extends SceneLayer<ClipRectLayerProperties> implements ui.ClipRectEngineLayer {
  ClipRectSceneLayer({ required super.properties, required super.oldId});
}

class ColorFilterSceneLayer extends SceneLayer<ColorFilterLayerProperties> implements ui.ColorFilterEngineLayer {
  ColorFilterSceneLayer({ required super.properties, required super.oldId});
}

class ImageFilterSceneLayer extends SceneLayer<ImageFilterLayerProperties> implements ui.ImageFilterEngineLayer {
  ImageFilterSceneLayer({ required super.properties, required super.oldId});
}

class OffsetSceneLayer extends SceneLayer<OffsetLayerProperties> implements ui.OffsetEngineLayer {
  OffsetSceneLayer({ required super.properties, required super.oldId});
}

class OpacitySceneLayer extends SceneLayer<OpacityLayerProperties> implements ui.OpacityEngineLayer {
  OpacitySceneLayer({ required super.properties, required super.oldId});
}

class PhysicalShapeSceneLayer extends SceneLayer<PhysicalShapeLayerProperties> implements ui.PhysicalShapeEngineLayer {
  PhysicalShapeSceneLayer({ required super.properties, required super.oldId});
}

class ShaderMaskSceneLayer extends SceneLayer<ShaderMaskLayerProperties> implements ui.ShaderMaskEngineLayer {
  ShaderMaskSceneLayer({ required super.properties, required super.oldId});
}

class TransformSceneLayer extends SceneLayer<TransformLayerProperties> implements ui.TransformEngineLayer {
  TransformSceneLayer({ required super.properties, required super.oldId});
}
