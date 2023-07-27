// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html';

import 'package:ui/ui.dart' as ui;
import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';

const String kCanvasContainerTag = 'flt-canvas-container';
const String kPlatformViewContainerTag = 'flt-platform-view';

class SliceContainer {
}

class SkwasmSceneView {
  factory SkwasmSceneView(SkwasmSurface surface) {
    final DomElement sceneElement = createDomElement('flt-scene');
    return SkwasmSceneView._(surface, sceneElement);
  }

  SkwasmSceneView._(this.surface, this.sceneElement);

  final SkwasmSurface surface;
  final DomElement sceneElement;

  int queuedRenders = 0;
  static const int kMaxQueuedRenders = 3;

  Future<void> renderScene(SkwasmScene scene) async {
    if (queuedRenders >= kMaxQueuedRenders) {
      return;
    }
    queuedRenders += 1;

    final List<LayerSlice> slices = scene.rootLayer.slices;
    final Iterable<Future<DomImageBitmap?>> renderFutures = slices.map(
      (LayerSlice slice) async => switch (slice) {
          PlatformViewSlice() => null,
          PictureSlice() => surface.renderPicture(slice.picture as SkwasmPicture),
        }
    );
    final List<DomImageBitmap?> renderedBitmaps = await Future.wait(renderFutures);

    // TODO: optimize this.
    DomElement? lastAdded;
    for (int i = 0; i < slices.length; i++) {
      final DomNode? nextNode = lastAdded == null 
        ? sceneElement.firstChild
        : lastAdded.nextSibling;
      final LayerSlice slice = slices[i];
      switch (slice) {
        case PictureSlice():
          final DomNode? candidate = findBestExistingPictureNode(slice);
          if (candidate != null) {
            if (nextNode != candidate) {
              sceneElement.removeChild(candidate);
              if (lastAdded == null) {
                sceneElement.prepend(candidate);
              } else {
                lastAdded.after()
              }
              sceneElement.prepend(
            }
          }
        case PlatformViewSlice():
          final 
      }
      final DomNode? candidate = lastAdded?.nextSibling ?? sceneElement.firstChild;

    }

    queuedRenders -= 1;
  }

  DomElement? findBestExistingPictureNode(PictureSlice slice) {
    final ui.Rect bounds = slice.picture.cullRect;
    for (DomElement? current = sceneElement.firstElementChild; current != null; current = current.nextSibling) {
      if (current.tagName != kCanvasContainerTag) {
        continue;
      }
      final DomCanvasElement canvas = current.firstChild! as DomCanvasElement;
      if (canvas.width == bounds.width && canvas.height == bounds.height) {
        return current;
      }
    }
    return null;
  }
}
