// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart';
import 'package:ui/src/engine/util.dart';
import 'package:ui/ui.dart' as ui;

import '../../engine.dart' show PlatformViewManager;
import '../vector_math.dart';
import 'embedded_views.dart';
import 'picture.dart';

/// A [Rendering] is a concrete description of how a Flutter scene will be
/// rendered in a web browser.
///
/// A [Rendering] is a sequence containing two types of entities:
///   * Render canvases: which contain rasterized CkPictures, and
///   * Platform views: being HTML content that is to be composited along with
///     the Flutter content.
class Rendering {
  final List<RenderingEntity> entities = <RenderingEntity>[];

  void add(RenderingEntity entity) {
    entities.add(entity);
  }
}

sealed class RenderingEntity {}

class RenderingRenderCanvas extends RenderingEntity {
  RenderingRenderCanvas({required this.requiredDueTo});
  final List<CkPicture> pictures = <CkPicture>[];

  /// The index of the platform view that caused this render canvas to be
  /// required.
  final int requiredDueTo;

  void add(CkPicture picture) {
    pictures.add(picture);
  }
}

class RenderingPlatformView extends RenderingEntity {
  RenderingPlatformView(this.viewId);
  final int viewId;
}

// Computes the bounds of the platform view from its associated parameters.
@visibleForTesting
ui.Rect computePlatformViewBounds(EmbeddedViewParams params) {
  ui.Rect currentClipBounds = ui.Rect.largest;
  Matrix4 currentTransform = Matrix4.identity();
  for (final Mutator mutator in params.mutators) {
    switch (mutator.type) {
      case MutatorType.clipRect:
        final ui.Rect transformedClipBounds =
            transformRectWithMatrix(currentTransform, mutator.rect!);
        currentClipBounds = currentClipBounds.intersect(transformedClipBounds);
      case MutatorType.clipRRect:
        final ui.Rect transformedClipBounds =
            transformRectWithMatrix(currentTransform, mutator.rrect!.outerRect);
        currentClipBounds = currentClipBounds.intersect(transformedClipBounds);
      case MutatorType.clipPath:
        final ui.Rect transformedClipBounds = transformRectWithMatrix(
            currentTransform, mutator.path!.getBounds());
        currentClipBounds.intersect(transformedClipBounds);
      case MutatorType.transform:
        currentTransform = currentTransform.multiplied(mutator.matrix!);
      case MutatorType.opacity:
        // Doesn't effect bounds.
        continue;
    }
  }
  final ui.Rect rawBounds = ui.Rect.fromLTWH(
    params.offset.dx,
    params.offset.dy,
    params.size.width,
    params.size.height,
  );
  final ui.Rect transformedBounds =
      transformRectWithMatrix(currentTransform, rawBounds);
  return transformedBounds.intersect(currentClipBounds);
}

Rendering createOptimizedRendering(
  List<CkPicture> pictures,
  List<int> platformViews,
  Map<int, EmbeddedViewParams> paramsForViews,
) {
  assert(pictures.length == platformViews.length + 1);
  final Map<CkPicture, int> overlayRequirement = <CkPicture, int>{};
  final Rendering result = Rendering();
  // The first render canvas is required due to the pseudo-platform view "V_0"
  // which is defined as a platform view that comes before all Flutter drawing
  // commands and intersects with everything.
  RenderingRenderCanvas currentRenderCanvas =
      RenderingRenderCanvas(requiredDueTo: 0);

  // This line essentially unwinds the first iteration of the following loop.
  // Since "V_0" intersects with all subsequent pictures, then the first picture
  // it intersects with is "P_0", so we create a new render canvas and add "P_0"
  // to it.
  currentRenderCanvas.add(pictures[0]);
  result.add(currentRenderCanvas);
  for (int i = 0; i < platformViews.length; i++) {
    result.add(RenderingPlatformView(platformViews[i]));
    // Find the first picture after this platform view that intersects with this
    // platform view.
    if (PlatformViewManager.instance.isVisible(platformViews[i])) {
      final ui.Rect platformViewBounds =
          computePlatformViewBounds(paramsForViews[platformViews[i]]!);
      for (final int j = i + 1; i < pictures.length; i++) {
        final ui.Rect pictureBounds = pictures[j].cullRect;
        if (platformViewBounds.overlaps(pictureBounds)) {
          overlayRequirement[pictures[j]] = i;
          break;
        }
      }
    }
    if (overlayRequirement.containsKey(pictures[i + 1]) &&
        overlayRequirement[pictures[i + 1]]! >=
            currentRenderCanvas.requiredDueTo) {
      currentRenderCanvas = RenderingRenderCanvas(
          requiredDueTo: overlayRequirement[pictures[i + 1]]!);
      currentRenderCanvas.add(pictures[i + 1]);
      result.add(currentRenderCanvas);
    } else {
      currentRenderCanvas.add(pictures[i + 1]);
    }
  }
  return result;
}
