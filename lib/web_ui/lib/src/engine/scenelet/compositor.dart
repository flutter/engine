// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../picture.dart';
import '../util.dart';
import '../vector_math.dart';
import 'scene.dart';
import 'scenelet.dart';

/// Composits the render trees and platform views of [Scenelet] objects into a
/// given [hostElement].
///
/// A compositor is a stateful long-lived object. One compositor should be
/// created for a given [FlutterView] and it should match that view's lifespan.
/// This object holds references to DOM elements that comprise the rendering of
/// of the view.
class SceneletCompositor {
  SceneletCompositor(this.hostElement);

  /// The element that hosts content rendered by this compositor's `FlutterView`.
  ///
  /// The compositor owns all descendants of this host element. No other parts
  /// of the engine should be mutating the contents of it to avoid confusion. If
  /// a new feature needs to be added that results in mutations inside this
  /// element, that feature should be expressed as part of this class, such as
  /// a new method, or new data in the scenelet graph that's implemented by this
  /// class.
  final DomElement hostElement;

  List<DomElement> _previousRendering = <DomElement>[];

  /// Updates the view to display the contents of the given [scene].
  void render(SceneletScene scene) {
    final List<DomElement> rendering = <DomElement>[];
    for (final Scenelet scenelet in scene.scenelets) {
      final List<SceneletPlatformView>? platformViews = scenelet.platformViews;
      if (platformViews != null) {
        for (final SceneletPlatformView platformView in platformViews) {
          final DomElement platformViewHost = createDomElement('flt-platform-view-host');
          platformViewHost.setAttribute('flt-view-id', platformView.viewId.toString());
          rendering.add(platformViewHost);
        }
      }

      final SceneletRenderTree? renderTree = scenelet.renderTree;
      if (renderTree != null) {
        final DomElement renderTreeHost = createDomElement('flt-render-tree-host');
        renderTreeHost.appendChild(_renderLayer(renderTree.rootLayer));
        rendering.add(renderTreeHost);
      }
    }

    for (final DomElement previousElement in _previousRendering) {
      previousElement.remove();
    }

    rendering.forEach(hostElement.append);

    _previousRendering = rendering;
  }

  DomElement _createLayerElement(String name) {
    final DomElement element = createDomElement(name);
    element.style
      ..transformOrigin = '0 0 0'
      ..position = 'absolute';
    return element;
  }

  DomElement _renderLayer(SceneletLayer layer) {
    final DomElement element;
    switch (layer.runtimeType) {
      case SceneletPictureLayer:
        layer as SceneletPictureLayer;
        final EnginePicture picture = layer.picture as EnginePicture;
        final ui.Rect bounds = picture.recordingCanvas!.pictureBounds!;
        element = _createLayerElement('flt-picture');
        element.style
          ..height = '${bounds.height}px'
          ..width = '${bounds.width}px'
          ..outline = '1px solid green'
          ..transform = 'translate(${bounds.left + layer.offset.dx}px, ${bounds.top + layer.offset.dy}px)';
        break;
      case SceneletTextureLayer:
        element = _createLayerElement('flt-texture');
        break;
      case PhysicalShapeSceneletLayer:
        element = _createLayerElement('flt-physical-shape');
        break;
      case ShaderMaskSceneletLayer:
        element = _createLayerElement('flt-shader-mask');
        break;
      case ClipRectSceneletLayer:
        element = _createLayerElement('flt-clip-rect');
        break;
      case ClipPathSceneletLayer:
        element = _createLayerElement('flt-clip-path');
        break;
      case ClipRRectSceneletLayer:
        element = _createLayerElement('flt-clip-rrect');
        break;
      case TransformSceneletLayer:
        layer as TransformSceneletLayer;
        element = _createLayerElement('flt-transform');
        setElementTransform(element, toMatrix32(layer.properties.matrix4));
        break;
      case BackdropFilterSceneletLayer:
        element = _createLayerElement('flt-backdrop-filter');
        break;
      case OpacitySceneletLayer:
        layer as OpacitySceneletLayer;
        element = _createLayerElement('flt-opacity');
        if (layer.properties.offset != ui.Offset.zero) {
          element.style.transform = 'translate(${layer.properties.offset.dx}px, ${layer.properties.offset.dy}px)';
        }
        break;
      case OffsetSceneletLayer:
        layer as OffsetSceneletLayer;
        element = _createLayerElement('flt-offset');
        element.style.transform = 'translate(${layer.properties.dx}px, ${layer.properties.dy}px)';
        break;
      case ColorFilterSceneletLayer:
        element = _createLayerElement('flt-color-filter');
        break;
      case ImageFilterSceneletLayer:
        layer as ImageFilterSceneletLayer;
        element = _createLayerElement('flt-image-filter');
        if (layer.properties.offset != ui.Offset.zero) {
          element.style.transform = 'translate(${layer.properties.offset.dx}px, ${layer.properties.offset.dy}px)';
        }
        break;
      case RootSceneletLayer:
        element = _createLayerElement('flt-root-layer');
        element.style.transform = 'scale(${1 / ui.window.devicePixelRatio})';
        break;
      default:
        throw StateError('${layer.runtimeType} is missing an implementation.');
    }

    if (layer is ContainerSceneletLayer) {
      for (final SceneletLayer child in layer.children) {
        element.appendChild(_renderLayer(child));
      }
    }

    return element;
  }
}
