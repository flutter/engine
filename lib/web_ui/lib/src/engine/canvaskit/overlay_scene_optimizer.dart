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

  List<RenderingRenderCanvas> get canvases =>
      entities.whereType<RenderingRenderCanvas>().toList();
}

/// An element of a [Rendering]. Either a render canvas or a platform view.
sealed class RenderingEntity {
  /// Returns [true] if this entity is equal to [other] for use in a rendering.
  ///
  /// For example, all [RenderingRenderCanvas] objects are equal to each other
  /// for purposes of rendering since any canvas in that place in the rendering
  /// will be equivalent. Platform views are only equal if they are for the same
  /// view id.
  bool equalForRendering(RenderingEntity other);
}

class RenderingRenderCanvas extends RenderingEntity {
  RenderingRenderCanvas({required this.requiredDueTo});

  /// The [pictures] which should be rendered in this canvas.
  final List<CkPicture> pictures = <CkPicture>[];

  /// The index of the platform view that caused this render canvas to be
  /// required.
  final int requiredDueTo;

  /// Adds the [picture] to the pictures that should be rendered in this canvas.
  void add(CkPicture picture) {
    pictures.add(picture);
  }

  @override
  bool equalForRendering(RenderingEntity other) {
    return other is RenderingRenderCanvas;
  }
}

/// A platform view to be rendered.
class RenderingPlatformView extends RenderingEntity {
  RenderingPlatformView(this.viewId);

  /// The [viewId] of the platform view to render.
  final int viewId;

  @override
  bool equalForRendering(RenderingEntity other) {
    return other is RenderingPlatformView && other.viewId == viewId;
  }
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

/// Returns the optimized [Rendering] for a sequence of [pictures] and
/// [platformViews].
///
/// [paramsForViews] is required to compute the bounds of the platform views.
Rendering createOptimizedRendering(
  List<CkPicture> pictures,
  List<int> platformViews,
  Map<int, EmbeddedViewParams> paramsForViews,
) {
  assert(pictures.length == platformViews.length + 1);

  final List<ui.Rect> pictureBounds = <ui.Rect>[];
  for (final CkPicture picture in pictures) {
    pictureBounds.add(picture.cullRect);
  }
  print(pictureBounds);

  final List<ui.Rect> platformViewBounds = <ui.Rect>[];
  for (final int viewId in platformViews) {
    platformViewBounds.add(computePlatformViewBounds(paramsForViews[viewId]!));
  }
  print(platformViewBounds);

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
      for (int j = i + 1; j < pictures.length; j++) {
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

/// Updates the DOM to display the [next] rendering by using the fewest
/// DOM operations to rearrange the [current] rendering.
void updateRendering(Rendering current, Rendering next) {}
