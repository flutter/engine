// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

const String kCanvasContainerTag = 'flt-canvas-container';

typedef RenderResult = ({
  List<DomImageBitmap> imageBitmaps,
  int rasterStartMicros,
  int rasterEndMicros,
});

// This is an interface that renders a `ScenePicture` as a `DomImageBitmap`.
// It is optionally asynchronous. It is required for the `EngineSceneView` to
// composite pictures into the canvases in the DOM tree it builds.
abstract class PictureRenderer {
  FutureOr<RenderResult> renderPictures(List<ScenePicture> picture);
  ScenePicture clipPicture(ScenePicture picture, ui.Rect clip);
}

class _SceneRender {
  _SceneRender(
    this.scene,
    this._completer, {
    this.recorder,
  }) {
    scene.beginRender();
  }

  final EngineScene scene;
  final Completer<void> _completer;
  final FrameTimingRecorder? recorder;

  void done() {
    scene.endRender();
    _completer.complete();
  }
}

// This class builds a DOM tree that composites an `EngineScene`.
class EngineSceneView {
  factory EngineSceneView(PictureRenderer pictureRenderer, ui.FlutterView flutterView) {
    final sceneElement = createDomElement('flt-scene');
    return EngineSceneView._(pictureRenderer, flutterView, sceneElement);
  }

  EngineSceneView._(this.pictureRenderer, this.flutterView, this.sceneElement);

  final PictureRenderer pictureRenderer;
  final DomElement sceneElement;
  final ui.FlutterView flutterView;

  List<SliceContainer> containers = <SliceContainer>[];

  _SceneRender? _currentRender;
  _SceneRender? _nextRender;

  Future<void> renderScene(EngineScene scene, FrameTimingRecorder? recorder) {
    if (_currentRender != null) {
      // If a scene is already queued up, drop it and queue this one up instead
      // so that the scene view always displays the most recently requested scene.
      _nextRender?.done();
      final completer = Completer<void>();
      _nextRender = _SceneRender(scene, completer, recorder: recorder);
      return completer.future;
    }
    final completer = Completer<void>();
    _currentRender = _SceneRender(scene, completer, recorder: recorder);
    _kickRenderLoop();
    return completer.future;
  }

  Future<void> _kickRenderLoop() async {
    final current = _currentRender!;
    await _renderScene(current.scene, current.recorder);
    current.done();
    _currentRender = _nextRender;
    _nextRender = null;
    if (_currentRender == null) {
      return;
    } else {
      return _kickRenderLoop();
    }
  }

  Future<void> _renderScene(EngineScene scene, FrameTimingRecorder? recorder) async {
    final screenBounds = ui.Rect.fromLTWH(
      0,
      0,
      flutterView.physicalSize.width,
      flutterView.physicalSize.height,
    );
    final slices = scene.rootLayer.slices;
    final picturesToRender = <ScenePicture>[];
    final originalPicturesToRender = <ScenePicture>[];
    for (final slice in slices) {
      if (slice is PictureSlice) {
        final clippedRect = slice.picture.cullRect.intersect(screenBounds);
        if (clippedRect.isEmpty) {
          // This picture is completely offscreen, so don't render it at all
          continue;
        } else if (clippedRect == slice.picture.cullRect) {
          // The picture doesn't need to be clipped, just render the original
          originalPicturesToRender.add(slice.picture);
          picturesToRender.add(slice.picture);
        } else {
          originalPicturesToRender.add(slice.picture);
          picturesToRender.add(pictureRenderer.clipPicture(slice.picture, clippedRect));
        }
      }
    }
    final Map<ScenePicture, DomImageBitmap> renderMap;
    if (picturesToRender.isNotEmpty) {
      final renderResult = await pictureRenderer.renderPictures(picturesToRender);
      renderMap = <ScenePicture, DomImageBitmap>{
        for (int i = 0; i < picturesToRender.length; i++)
          originalPicturesToRender[i]: renderResult.imageBitmaps[i],
      };
      recorder?.recordRasterStart(renderResult.rasterStartMicros);
      recorder?.recordRasterFinish(renderResult.rasterEndMicros);
    } else {
      renderMap = <ScenePicture, DomImageBitmap>{};
      recorder?.recordRasterStart();
      recorder?.recordRasterFinish();
    }
    recorder?.submitTimings();

    final reusableContainers = List<SliceContainer?>.from(containers);
    final newContainers = <SliceContainer>[];
    for (final slice in slices) {
      switch (slice) {
        case PictureSlice():
          final bitmap = renderMap[slice.picture];
          if (bitmap == null) {
            // We didn't render this slice because no part of it is visible.
            continue;
          }
          PictureSliceContainer? container;
          for (var j = 0; j < reusableContainers.length; j++) {
            final candidate = reusableContainers[j];
            if (candidate is PictureSliceContainer) {
              container = candidate;
              reusableContainers[j] = null;
              break;
            }
          }

          final clippedBounds = slice.picture.cullRect.intersect(screenBounds);
          if (container != null) {
            container.bounds = clippedBounds;
          } else {
            container = PictureSliceContainer(clippedBounds);
          }
          container.updateContents();
          container.renderBitmap(bitmap);
          newContainers.add(container);

        case PlatformViewSlice():
          for (final view in slice.views) {
            // TODO(harryterkelsen): Inject the FlutterView instance from `renderScene`,
            // instead of using `EnginePlatformDispatcher...implicitView` directly,
            // or make the FlutterView "register" like in canvaskit.
            // Ensure the platform view contents are injected in the DOM.
            EnginePlatformDispatcher.instance.implicitView?.dom.injectPlatformView(view.viewId);

            // Attempt to reuse a container for the existing view
            PlatformViewContainer? container;
            for (var j = 0; j < reusableContainers.length; j++) {
              final candidate = reusableContainers[j];
              if (candidate is PlatformViewContainer && candidate.viewId == view.viewId) {
                container = candidate;
                reusableContainers[j] = null;
                break;
              }
            }
            container ??= PlatformViewContainer(view.viewId);
            container.size = view.size;
            container.styling = view.styling;
            container.updateContents();
            newContainers.add(container);
          }
      }
    }

    containers = newContainers;

    var currentElement = sceneElement.firstElementChild;
    for (final container in containers) {
      if (currentElement == null) {
        sceneElement.appendChild(container.container);
      } else if (currentElement == container.container) {
        currentElement = currentElement.nextElementSibling;
      } else {
        sceneElement.insertBefore(container.container, currentElement);
      }
    }

    // Remove any other unused containers
    while (currentElement != null) {
      final sibling = currentElement.nextElementSibling;
      sceneElement.removeChild(currentElement);
      currentElement = sibling;
    }
  }
}

sealed class SliceContainer {
  DomElement get container;

  void updateContents();
}

final class PictureSliceContainer extends SliceContainer {
  factory PictureSliceContainer(ui.Rect bounds) {
    final container = domDocument.createElement(kCanvasContainerTag);
    final canvas = createDomCanvasElement(
      width: bounds.width.toInt(),
      height: bounds.height.toInt()
    );
    container.appendChild(canvas);
    return PictureSliceContainer._(bounds, container, canvas);
  }

  PictureSliceContainer._(this._bounds, this.container, this.canvas);

  ui.Rect _bounds;
  bool _dirty = true;

  ui.Rect get bounds => _bounds;
  set bounds(ui.Rect bounds) {
    if (_bounds != bounds) {
      _bounds = bounds;
      _dirty = true;
    }
  }

  @override
  void updateContents() {
    if (_dirty) {
      _dirty = false;

      final roundedOutBounds = ui.Rect.fromLTRB(
        bounds.left.floorToDouble(),
        bounds.top.floorToDouble(),
        bounds.right.ceilToDouble(),
        bounds.bottom.ceilToDouble()
      );
      final style = canvas.style;
      final devicePixelRatio = EngineFlutterDisplay.instance.devicePixelRatio;
      final logicalWidth = roundedOutBounds.width / devicePixelRatio;
      final logicalHeight = roundedOutBounds.height / devicePixelRatio;
      final logicalLeft = roundedOutBounds.left / devicePixelRatio;
      final logicalTop = roundedOutBounds.top / devicePixelRatio;
      style.width = '${logicalWidth}px';
      style.height = '${logicalHeight}px';
      style.position = 'absolute';
      style.left = '${logicalLeft}px';
      style.top = '${logicalTop}px';
      canvas.width = roundedOutBounds.width.ceilToDouble();
      canvas.height = roundedOutBounds.height.ceilToDouble();
    }
  }

  void renderBitmap(DomImageBitmap bitmap) {
    final ctx = canvas.contextBitmapRenderer;
    ctx.transferFromImageBitmap(bitmap);
  }

  @override
  final DomElement container;
  final DomCanvasElement canvas;
}

final class PlatformViewContainer extends SliceContainer {
  PlatformViewContainer(this.viewId) : container = createPlatformViewSlot(viewId);

  final int viewId;
  PlatformViewStyling? _styling;
  ui.Size? _size;
  bool _dirty = false;

  @override
  final DomElement container;

  set styling(PlatformViewStyling styling) {
    if (_styling != styling) {
      _styling = styling;
      _dirty = true;
    }
  }

  set size(ui.Size size) {
    if (_size != size) {
      _size = size;
      _dirty = true;
    }
  }


  @override
  void updateContents() {
    assert(_styling != null);
    assert(_size != null);
    if (_dirty) {
      final style = container.style;
      final devicePixelRatio = EngineFlutterDisplay.instance.devicePixelRatio;
      final logicalWidth = _size!.width / devicePixelRatio;
      final logicalHeight = _size!.height / devicePixelRatio;
      style.width = '${logicalWidth}px';
      style.height = '${logicalHeight}px';
      style.position = 'absolute';

      final offset = _styling!.position.offset;
      final logicalLeft = (offset?.dx ?? 0) / devicePixelRatio;
      final logicalTop = (offset?.dy ?? 0) / devicePixelRatio;
      style.left = '${logicalLeft}px';
      style.top = '${logicalTop}px';

      final transform = _styling!.position.transform;
      style.transform = transform != null ? float64ListToCssTransform3d(transform.storage) : '';
      style.opacity = _styling!.opacity != 1.0 ? '${_styling!.opacity}' : '';
      // TODO(jacksongardner): Implement clip styling for platform views

      _dirty = false;
    }
  }
}
