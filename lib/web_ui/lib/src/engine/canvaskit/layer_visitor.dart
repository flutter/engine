// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../color_filter.dart';
import '../vector_math.dart';
import 'canvas.dart';
import 'canvaskit_api.dart';
import 'color_filter.dart';
import 'embedded_views.dart';
import 'image_filter.dart';
import 'layer.dart';
import 'n_way_canvas.dart';
import 'painting.dart';
import 'picture.dart';

abstract class LayerVisitor<T> {
  void visitRoot(RootLayer root, T childData);
  void visitBackdropFilter(
      BackdropFilterEngineLayer backdropFilter, T childData);
  void visitClipPath(ClipPathEngineLayer clipPath, T childData);
  void visitClipRect(ClipRectEngineLayer clipRect, T childData);
  void visitClipRRect(ClipRRectEngineLayer clipRRect, T childData);
  void visitOpacity(OpacityEngineLayer opacity, T childData);
  void visitTransform(TransformEngineLayer transform, T childData);
  void visitOffset(OffsetEngineLayer offset, T childData);
  void visitImageFilter(ImageFilterEngineLayer imageFilter, T childData);
  void visitShaderMask(ShaderMaskEngineLayer shaderMask, T childData);
  void visitPicture(PictureLayer picture, T childData);
  void visitColorFilter(ColorFilterEngineLayer colorFilter, T childData);
  void visitPlatformView(PlatformViewLayer platformView, T childData);
}

/// Pre-process the layer tree before painting.
///
/// In this step, we compute the estimated [paintBounds] as well as
/// apply heuristics to prepare the render cache for pictures that
/// should be cached.
class PrerollVisitor extends LayerVisitor<Matrix4> {
  PrerollVisitor(this.viewEmbedder);

  final MutatorsStack mutatorsStack = MutatorsStack();

  /// A compositor for embedded HTML views.
  final HtmlViewEmbedder? viewEmbedder;

  /// How many layers deep of filter masks we are. We cannot split pictures
  /// that are affected by a shader mask or a backdrop filter.
  int filterMaskLayers = 0;

  CkPicture? lastPicture;

  ui.Rect get cullRect {
    ui.Rect cullRect = ui.Rect.largest;
    for (final Mutator m in mutatorsStack) {
      ui.Rect clipRect;
      switch (m.type) {
        case MutatorType.clipRect:
          clipRect = m.rect!;
        case MutatorType.clipRRect:
          clipRect = m.rrect!.outerRect;
        case MutatorType.clipPath:
          clipRect = m.path!.getBounds();
        default:
          continue;
      }
      cullRect = cullRect.intersect(clipRect);
    }
    return cullRect;
  }

  /// Run [preroll] on all of the child layers.
  ///
  /// Returns a [Rect] that covers the paint bounds of all of the child layers.
  /// If all of the child layers have empty paint bounds, then the returned
  /// [Rect] is empty.
  ui.Rect prerollChildren(ContainerLayer layer, Matrix4 childMatrix) {
    ui.Rect childPaintBounds = ui.Rect.zero;
    for (final Layer layer in layer.children) {
      layer.accept(this, childMatrix);
      if (childPaintBounds.isEmpty) {
        childPaintBounds = layer.paintBounds;
      } else if (!layer.paintBounds.isEmpty) {
        childPaintBounds = childPaintBounds.expandToInclude(layer.paintBounds);
      }
    }
    return childPaintBounds;
  }

  void prerollContainerLayer(ContainerLayer container, Matrix4 matrix) {
    container.paintBounds = prerollChildren(container, matrix);
  }

  @override
  void visitRoot(RootLayer root, Matrix4 matrix) {
    prerollContainerLayer(root, matrix);
  }

  @override
  void visitBackdropFilter(
      BackdropFilterEngineLayer backdropFilter, Matrix4 matrix) {
    final ui.Rect childBounds = prerollChildren(backdropFilter, matrix);
    backdropFilter.paintBounds = childBounds.expandToInclude(cullRect);
  }

  @override
  void visitClipPath(ClipPathEngineLayer clipPath, Matrix4 matrix) {
    mutatorsStack.pushClipPath(clipPath.clipPath);
    final ui.Rect childPaintBounds = prerollChildren(clipPath, matrix);
    final ui.Rect clipBounds = clipPath.clipPath.getBounds();
    if (childPaintBounds.overlaps(clipBounds)) {
      clipPath.paintBounds = childPaintBounds.intersect(clipBounds);
    }
    mutatorsStack.pop();
  }

  @override
  void visitClipRRect(ClipRRectEngineLayer clipRRect, Matrix4 matrix) {
    mutatorsStack.pushClipRRect(clipRRect.clipRRect);
    final ui.Rect childPaintBounds = prerollChildren(clipRRect, matrix);
    if (childPaintBounds.overlaps(clipRRect.clipRRect.outerRect)) {
      clipRRect.paintBounds =
          childPaintBounds.intersect(clipRRect.clipRRect.outerRect);
    }
    mutatorsStack.pop();
  }

  @override
  void visitClipRect(ClipRectEngineLayer clipRect, Matrix4 matrix) {
    mutatorsStack.pushClipRect(clipRect.clipRect);
    final ui.Rect childPaintBounds = prerollChildren(clipRect, matrix);
    if (childPaintBounds.overlaps(clipRect.clipRect)) {
      clipRect.paintBounds = childPaintBounds.intersect(clipRect.clipRect);
    }
    mutatorsStack.pop();
  }

  @override
  void visitColorFilter(ColorFilterEngineLayer colorFilter, Matrix4 matrix) {
    prerollContainerLayer(colorFilter, matrix);
  }

  @override
  void visitImageFilter(ImageFilterEngineLayer imageFilter, Matrix4 matrix) {
    final Matrix4 childMatrix = Matrix4.copy(matrix);
    childMatrix.translate(imageFilter.offset.dx, imageFilter.offset.dy);
    mutatorsStack.pushTransform(Matrix4.translationValues(
        imageFilter.offset.dx, imageFilter.offset.dy, 0.0));
    final CkManagedSkImageFilterConvertible convertible;
    if (imageFilter.filter is ui.ColorFilter) {
      convertible =
          createCkColorFilter(imageFilter.filter as EngineColorFilter)!;
    } else {
      convertible = imageFilter.filter as CkManagedSkImageFilterConvertible;
    }
    ui.Rect childPaintBounds = prerollChildren(imageFilter, childMatrix);
    childPaintBounds = childPaintBounds.translate(
        imageFilter.offset.dx, imageFilter.offset.dy);
    if (imageFilter.filter is ui.ColorFilter) {
      // If the filter is a ColorFilter, the extended paint bounds will be the
      // entire screen, which is not what we want.
      imageFilter.paintBounds = childPaintBounds;
    } else {
      convertible.withSkImageFilter((SkImageFilter skFilter) {
        imageFilter.paintBounds = rectFromSkIRect(
          skFilter.getOutputBounds(toSkRect(childPaintBounds)),
        );
      });
    }
    mutatorsStack.pop();
  }

  @override
  void visitOffset(OffsetEngineLayer offset, Matrix4 matrix) {
    visitTransform(offset, matrix);
  }

  @override
  void visitOpacity(OpacityEngineLayer opacity, Matrix4 matrix) {
    final Matrix4 childMatrix = Matrix4.copy(matrix);
    childMatrix.translate(opacity.offset.dx, opacity.offset.dy);
    mutatorsStack.pushTransform(
        Matrix4.translationValues(opacity.offset.dx, opacity.offset.dy, 0.0));
    mutatorsStack.pushOpacity(opacity.alpha);
    prerollContainerLayer(opacity, childMatrix);
    mutatorsStack.pop();
    mutatorsStack.pop();
    opacity.paintBounds =
        opacity.paintBounds.translate(opacity.offset.dx, opacity.offset.dy);
  }

  @override
  void visitPicture(PictureLayer picture, Matrix4 matrix) {
    picture.paintBounds = picture.picture.cullRect.shift(picture.offset);
    if (filterMaskLayers == 0) {
      viewEmbedder?.prerollPicture(picture.picture);
    } else {
      lastPicture = picture.picture;
    }
  }

  @override
  void visitPlatformView(PlatformViewLayer platformView, Matrix4 matrix) {
    platformView.paintBounds = ui.Rect.fromLTWH(
      platformView.offset.dx,
      platformView.offset.dy,
      platformView.width,
      platformView.height,
    );

    /// ViewEmbedder is set to null when screenshotting. Therefore, skip
    /// rendering
    viewEmbedder?.prerollCompositeEmbeddedView(
      platformView.viewId,
      EmbeddedViewParams(
        platformView.offset,
        ui.Size(platformView.width, platformView.height),
        mutatorsStack,
      ),
    );
  }

  @override
  void visitShaderMask(ShaderMaskEngineLayer shaderMask, Matrix4 matrix) {
    filterMaskLayers++;
    shaderMask.paintBounds = prerollChildren(shaderMask, matrix);
    filterMaskLayers--;
    if (filterMaskLayers == 0) {
      if (lastPicture != null) {
        viewEmbedder?.prerollPicture(lastPicture!);
      }
    }
  }

  @override
  void visitTransform(TransformEngineLayer transform, Matrix4 matrix) {
    final Matrix4 childMatrix = matrix.multiplied(transform.transform);
    mutatorsStack.pushTransform(transform.transform);
    final ui.Rect childPaintBounds = prerollChildren(transform, childMatrix);
    transform.paintBounds = transform.transform.transformRect(childPaintBounds);
    mutatorsStack.pop();
  }
}

class PaintVisitor extends LayerVisitor<void> {
  PaintVisitor(
    this.internalNodesCanvas,
    this.leafNodesCanvas,
    this.viewEmbedder,
  );

  /// A multi-canvas that applies clips, transforms, and opacity
  /// operations to all canvases (root canvas and overlay canvases for the
  /// platform views).
  CkNWayCanvas internalNodesCanvas;

  /// The canvas for leaf nodes to paint to.
  CkCanvas? leafNodesCanvas;

  /// A compositor for embedded HTML views.
  final HtmlViewEmbedder? viewEmbedder;

  /// How many layers deep of filter masks we are. We cannot split pictures
  /// that are children of the same shader mask or backdrop filter.
  int filterMaskLayers = 0;

  /// Calls [paint] on all child layers that need painting.
  void paintChildren(ContainerLayer container) {
    assert(container.needsPainting);

    for (final Layer layer in container.children) {
      if (layer.needsPainting) {
        layer.accept(this, null);
      }
    }
  }

  @override
  void visitRoot(RootLayer root, _) {
    paintChildren(root);
  }

  @override
  void visitBackdropFilter(BackdropFilterEngineLayer backdropFilter, _) {
    final CkPaint paint = CkPaint()..blendMode = backdropFilter.blendMode;

    // Only apply the backdrop filter to the current canvas. If we apply the
    // backdrop filter to every canvas (i.e. by applying it to the
    // [internalNodesCanvas]), then later when we compose the canvases into a
    // single canvas, the backdrop filter will be applied multiple times.
    final CkCanvas currentCanvas = leafNodesCanvas!;
    currentCanvas.saveLayerWithFilter(
        backdropFilter.paintBounds, backdropFilter.filter, paint);
    paintChildren(backdropFilter);
    currentCanvas.restore();
  }

  @override
  void visitClipPath(ClipPathEngineLayer clipPath, _) {
    assert(clipPath.needsPainting);

    internalNodesCanvas.save();
    internalNodesCanvas.clipPath(
        clipPath.clipPath, clipPath.clipBehavior != ui.Clip.hardEdge);

    if (clipPath.clipBehavior == ui.Clip.antiAliasWithSaveLayer) {
      internalNodesCanvas.saveLayer(clipPath.paintBounds, null);
    }
    paintChildren(clipPath);
    if (clipPath.clipBehavior == ui.Clip.antiAliasWithSaveLayer) {
      internalNodesCanvas.restore();
    }
    internalNodesCanvas.restore();
  }

  @override
  void visitClipRect(ClipRectEngineLayer clipRect, _) {
    assert(clipRect.needsPainting);

    internalNodesCanvas.save();
    internalNodesCanvas.clipRect(
      clipRect.clipRect,
      ui.ClipOp.intersect,
      clipRect.clipBehavior != ui.Clip.hardEdge,
    );
    if (clipRect.clipBehavior == ui.Clip.antiAliasWithSaveLayer) {
      internalNodesCanvas.saveLayer(clipRect.clipRect, null);
    }
    paintChildren(clipRect);
    if (clipRect.clipBehavior == ui.Clip.antiAliasWithSaveLayer) {
      internalNodesCanvas.restore();
    }
    internalNodesCanvas.restore();
  }

  @override
  void visitClipRRect(ClipRRectEngineLayer clipRRect, _) {
    assert(clipRRect.needsPainting);

    internalNodesCanvas.save();
    internalNodesCanvas.clipRRect(
        clipRRect.clipRRect, clipRRect.clipBehavior != ui.Clip.hardEdge);
    if (clipRRect.clipBehavior == ui.Clip.antiAliasWithSaveLayer) {
      internalNodesCanvas.saveLayer(clipRRect.paintBounds, null);
    }
    paintChildren(clipRRect);
    if (clipRRect.clipBehavior == ui.Clip.antiAliasWithSaveLayer) {
      internalNodesCanvas.restore();
    }
    internalNodesCanvas.restore();
  }

  @override
  void visitOpacity(OpacityEngineLayer opacity, _) {
    assert(opacity.needsPainting);

    final CkPaint paint = CkPaint();
    paint.color = ui.Color.fromARGB(opacity.alpha, 0, 0, 0);

    internalNodesCanvas.save();
    internalNodesCanvas.translate(opacity.offset.dx, opacity.offset.dy);

    final ui.Rect saveLayerBounds = opacity.paintBounds.shift(-opacity.offset);

    internalNodesCanvas.saveLayer(saveLayerBounds, paint);
    paintChildren(opacity);
    // Restore twice: once for the translate and once for the saveLayer.
    internalNodesCanvas.restore();
    internalNodesCanvas.restore();
  }

  @override
  void visitTransform(TransformEngineLayer transform, _) {
    assert(transform.needsPainting);

    internalNodesCanvas.save();
    internalNodesCanvas.transform(transform.transform.storage);
    paintChildren(transform);
    internalNodesCanvas.restore();
  }

  @override
  void visitOffset(OffsetEngineLayer offset, _) {
    visitTransform(offset, null);
  }

  @override
  void visitImageFilter(ImageFilterEngineLayer imageFilter, _) {
    assert(imageFilter.needsPainting);
    final ui.Rect offsetPaintBounds =
        imageFilter.paintBounds.shift(-imageFilter.offset);
    internalNodesCanvas.save();
    internalNodesCanvas.translate(imageFilter.offset.dx, imageFilter.offset.dy);
    internalNodesCanvas.clipRect(offsetPaintBounds, ui.ClipOp.intersect, false);
    final CkPaint paint = CkPaint();
    paint.imageFilter = imageFilter.filter;
    internalNodesCanvas.saveLayer(offsetPaintBounds, paint);
    paintChildren(imageFilter);
    internalNodesCanvas.restore();
    internalNodesCanvas.restore();
  }

  @override
  void visitShaderMask(ShaderMaskEngineLayer shaderMask, _) {
    assert(shaderMask.needsPainting);

    filterMaskLayers++;
    internalNodesCanvas.saveLayer(shaderMask.paintBounds, null);
    paintChildren(shaderMask);

    final CkPaint paint = CkPaint();
    paint.shader = shaderMask.shader;
    paint.blendMode = shaderMask.blendMode;
    paint.filterQuality = shaderMask.filterQuality;

    leafNodesCanvas!.save();
    leafNodesCanvas!
        .translate(shaderMask.maskRect.left, shaderMask.maskRect.top);

    leafNodesCanvas!.drawRect(
        ui.Rect.fromLTWH(
            0, 0, shaderMask.maskRect.width, shaderMask.maskRect.height),
        paint);
    leafNodesCanvas!.restore();
    filterMaskLayers--;

    if (filterMaskLayers == 0) {
      final CkCanvas? nextCanvas = viewEmbedder?.finalizePicture();
      if (nextCanvas != null) {
        leafNodesCanvas = nextCanvas;
      }
    }

    internalNodesCanvas.restore();
  }

  @override
  void visitPicture(PictureLayer picture, _) {
    assert(picture.needsPainting);

    leafNodesCanvas!.save();
    leafNodesCanvas!.translate(picture.offset.dx, picture.offset.dy);

    leafNodesCanvas!.drawPicture(picture.picture);
    leafNodesCanvas!.restore();

    if (filterMaskLayers == 0) {
      final CkCanvas? nextCanvas =
          viewEmbedder?.finalizePicture(picture.picture);
      if (nextCanvas != null) {
        leafNodesCanvas = nextCanvas;
      }
    }
  }

  @override
  void visitColorFilter(ColorFilterEngineLayer colorFilter, _) {
    assert(colorFilter.needsPainting);

    final CkPaint paint = CkPaint();
    paint.colorFilter = colorFilter.filter;

    // We need to clip because if the ColorFilter affects transparent black,
    // then it will fill the entire `cullRect` of the picture, ignoring the
    // `paintBounds` passed to `saveLayer`. See:
    // https://github.com/flutter/flutter/issues/88866
    internalNodesCanvas.save();

    // TODO(hterkelsen): Only clip if the ColorFilter affects transparent black.
    internalNodesCanvas.clipRect(
        colorFilter.paintBounds, ui.ClipOp.intersect, false);

    internalNodesCanvas.saveLayer(colorFilter.paintBounds, paint);
    paintChildren(colorFilter);
    internalNodesCanvas.restore();
    internalNodesCanvas.restore();
  }

  @override
  void visitPlatformView(PlatformViewLayer platformView, _) {
    // TODO(harryterkelsen): Warn if we are a child of a backdrop filter or
    // shader mask.
    viewEmbedder?.compositeEmbeddedView(platformView.viewId);
  }
}
