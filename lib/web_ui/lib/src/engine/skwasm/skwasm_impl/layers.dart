// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:ui/src/engine/scene_painting.dart';
import 'package:ui/src/engine/vector_math.dart';
import 'package:ui/ui.dart' as ui;

class RootLayer with PictureLayer {}

class BackdropFilterLayer
  with PictureLayer
  implements ui.BackdropFilterEngineLayer {}
class BackdropFilterOperation implements LayerOperation {
  BackdropFilterOperation(this.filter, this.mode);

  final ui.ImageFilter filter;
  final ui.BlendMode mode;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect;

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.saveLayerWithFilter(contentRect, ui.Paint()..blendMode = mode, filter);
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() => const PlatformViewStyling();
}

class ClipPathLayer
  with PictureLayer
  implements ui.ClipPathEngineLayer {}
class ClipPathOperation implements LayerOperation {
  ClipPathOperation(this.path, this.clip);

  final ui.Path path;
  final ui.Clip clip;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect.intersect(path.getBounds());

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.save();
    canvas.clipPath(path, doAntiAlias: clip != ui.Clip.hardEdge);
    if (clip == ui.Clip.antiAliasWithSaveLayer) {
      canvas.saveLayer(path.getBounds(), ui.Paint());
    }
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    if (clip == ui.Clip.antiAliasWithSaveLayer) {
      canvas.restore();
    }
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() {
    // TODO: implement this
    return const PlatformViewStyling();
  }
}

class ClipRectLayer
  with PictureLayer
  implements ui.ClipRectEngineLayer {}
class ClipRectOperation implements LayerOperation {
  const ClipRectOperation(this.rect, this.clip);

  final ui.Rect rect;
  final ui.Clip clip;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect.intersect(rect);

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.save();
    canvas.clipRect(rect, doAntiAlias: clip != ui.Clip.hardEdge);
    if (clip == ui.Clip.antiAliasWithSaveLayer) {
      canvas.saveLayer(rect, ui.Paint());
    }
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    if (clip == ui.Clip.antiAliasWithSaveLayer) {
      canvas.restore();
    }
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() {
    // TODO: implement createPlatformViewStyling
    return const PlatformViewStyling();
  }
}

class ClipRRectLayer
  with PictureLayer
  implements ui.ClipRRectEngineLayer {}
class ClipRRectOperation implements LayerOperation {
  const ClipRRectOperation(this.rrect, this.clip);

  final ui.RRect rrect;
  final ui.Clip clip;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect.intersect(rrect.outerRect);

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.save();
    canvas.clipRRect(rrect, doAntiAlias: clip != ui.Clip.hardEdge);
    if (clip == ui.Clip.antiAliasWithSaveLayer) {
      canvas.saveLayer(rrect.outerRect, ui.Paint());
    }
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    if (clip == ui.Clip.antiAliasWithSaveLayer) {
      canvas.restore();
    }
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() {
    // TODO: implement createPlatformViewStyling
    return const PlatformViewStyling();
  }
}

class ColorFilterLayer
  with PictureLayer
  implements ui.ColorFilterEngineLayer {}
class ColorFilterOperation implements LayerOperation {
  ColorFilterOperation(this.filter);

  final ui.ColorFilter filter;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect;

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.saveLayer(contentRect, ui.Paint()..colorFilter = filter);
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() => const PlatformViewStyling();
}

class ImageFilterLayer
  with PictureLayer
  implements ui.ImageFilterEngineLayer {}
class ImageFilterOperation implements LayerOperation {
  ImageFilterOperation(this.filter, this.offset);

  final ui.ImageFilter filter;
  final ui.Offset offset;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect;

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    if (offset != ui.Offset.zero) {
      canvas.save();
      canvas.translate(offset.dx, offset.dy);
    }
    final ui.Rect adjustedContentRect =
      (filter as SceneImageFilter).filterBounds(contentRect);
    canvas.saveLayer(adjustedContentRect, ui.Paint()..imageFilter = filter);
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    if (offset != ui.Offset.zero) {
      canvas.restore();
    }
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() {
    if (offset != ui.Offset.zero) {
      return PlatformViewStyling(
        position: PlatformViewPosition(offset: offset)
      );
    } else {
      return const PlatformViewStyling();
    }
  }
}

class OffsetLayer
  with PictureLayer
  implements ui.OffsetEngineLayer {}
class OffsetOperation implements LayerOperation {
  OffsetOperation(this.dx, this.dy);

  final double dx;
  final double dy;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect.shift(ui.Offset(dx, dy));

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect.shift(ui.Offset(-dx, -dy));

  @override
  void pre(SceneCanvas canvas, ui.Rect cullRect) {
    canvas.save();
    canvas.translate(dx, dy);
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() => PlatformViewStyling(
    position: PlatformViewPosition(offset: ui.Offset(dx, dy))
  );
}

class OpacityLayer
  with PictureLayer
  implements ui.OpacityEngineLayer {}
class OpacityOperation implements LayerOperation {
  OpacityOperation(this.alpha, this.offset);

  final int alpha;
  final ui.Offset offset;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect.shift(offset);

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect cullRect) {
    if (offset != ui.Offset.zero) {
      canvas.save();
      canvas.translate(offset.dx, offset.dy);
    }
    canvas.saveLayer(
      cullRect,
      ui.Paint()..color = ui.Color.fromARGB(alpha, 0, 0, 0)
    );
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.restore();
    if (offset != ui.Offset.zero) {
      canvas.restore();
    }
  }

  @override
  PlatformViewStyling createPlatformViewStyling() => PlatformViewStyling(
    position: offset != ui.Offset.zero ? PlatformViewPosition(offset: offset) : const PlatformViewPosition(),
    opacity: alpha.toDouble() / 255.0,
  );
}

class TransformLayer
  with PictureLayer
  implements ui.TransformEngineLayer {}
class TransformOperation implements LayerOperation {
  TransformOperation(this.transform);

  final Float64List transform;

  Matrix4 getMatrix() => Matrix4.fromFloat32List(toMatrix32(transform));

  @override
  ui.Rect cullRect(ui.Rect contentRect) => getMatrix().transformRect(contentRect);

  @override
  ui.Rect inverseMapRect(ui.Rect rect) {
    final Matrix4 matrix = getMatrix()..invert();
    return matrix.transformRect(rect);
  }

  @override
  void pre(SceneCanvas canvas, ui.Rect cullRect) {
    canvas.save();
    canvas.transform(transform);
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() => PlatformViewStyling(
    position: PlatformViewPosition(transform: getMatrix()),
  );
}

class ShaderMaskLayer
  with PictureLayer
  implements ui.ShaderMaskEngineLayer {}
class ShaderMaskOperation implements LayerOperation {
  ShaderMaskOperation(this.shader, this.maskRect, this.blendMode);

  final ui.Shader shader;
  final ui.Rect maskRect;
  final ui.BlendMode blendMode;

  @override
  ui.Rect cullRect(ui.Rect contentRect) => contentRect;

  @override
  ui.Rect inverseMapRect(ui.Rect rect) => rect;

  @override
  void pre(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.saveLayer(
      contentRect,
      ui.Paint(),
    );
  }

  @override
  void post(SceneCanvas canvas, ui.Rect contentRect) {
    canvas.save();
    canvas.translate(maskRect.left, maskRect.top);
    canvas.drawRect(
      ui.Rect.fromLTWH(0, 0, maskRect.width, maskRect.height),
      ui.Paint()
        ..blendMode = blendMode
        ..shader = shader
    );
    canvas.restore();
    canvas.restore();
  }

  @override
  PlatformViewStyling createPlatformViewStyling() {
    // TODO: implement createPlatformViewStyling
    return const PlatformViewStyling();
  }
}

class PlatformView {
  PlatformView(this.viewId, this.size, this.styling);

  int viewId;

  // The bounds of this platform view, in the layer's local coordinate space.
  ui.Size size;

  PlatformViewStyling styling;
}

sealed class LayerSlice {
  void dispose();
}

class PlatformViewSlice implements LayerSlice {
  PlatformViewSlice(this.views, this.occlusionRect);

  List<PlatformView> views;

  // A conservative estimate of what area platform views in this slice may cover.
  // This is expressed in the coordinate space of the parent.
  ui.Rect? occlusionRect;

  @override
  void dispose() {}
}

class PictureSlice implements LayerSlice {
  PictureSlice(this.picture);

  ScenePicture picture;

  @override
  void dispose() => picture.dispose();

  @override
  String toString() {
    return 'PictureSlice(${picture.cullRect})';
  }
}

mixin PictureLayer implements ui.EngineLayer {
  List<LayerSlice> slices = <LayerSlice>[];

  @override
  void dispose() {
    for (final LayerSlice slice in slices) {
      slice.dispose();
    }
  }
}

abstract class LayerOperation {
  const LayerOperation();

  ui.Rect cullRect(ui.Rect contentRect);

  // Takes a rectangle in the layer's coordinate space and maps it to the parent
  // coordinate space.
  ui.Rect inverseMapRect(ui.Rect rect);
  void pre(SceneCanvas canvas, ui.Rect contentRect);
  void post(SceneCanvas canvas, ui.Rect contentRect);

  PlatformViewStyling createPlatformViewStyling();
}

class PictureDrawCommand {
  PictureDrawCommand(this.offset, this.picture);

  ui.Offset offset;
  ui.Picture picture;
}

class PlatformViewPosition {
  const PlatformViewPosition({this.offset, this.transform});
  const PlatformViewPosition.zero() : offset = null, transform = null;

  final ui.Offset? offset;
  final Matrix4? transform;

  static PlatformViewPosition combine(PlatformViewPosition outer, PlatformViewPosition inner) {
    final ui.Offset? outerOffset = outer.offset;
    final Matrix4? outerTransform = outer.transform;
    final ui.Offset? innerOffset = inner.offset;
    final Matrix4? innerTransform = inner.transform;
    if (innerTransform != null) {
      if (innerOffset != null) {
        if (outerTransform != null) {
          final Matrix4 newTransform = innerTransform.clone();
          newTransform.translate(innerOffset.dx, innerOffset.dy);
          newTransform.multiply(outerTransform);
          return PlatformViewPosition(offset: outerOffset, transform: newTransform);
        } else {
          final ui.Offset finalOffset = outerOffset != null ? (innerOffset + outerOffset) : innerOffset;
          return PlatformViewPosition(offset: finalOffset, transform: innerTransform);
        }
      } else {
        if (outerTransform != null) {
          final Matrix4 newTransform = innerTransform.clone();
          newTransform.multiply(outerTransform);
          return PlatformViewPosition(offset: outerOffset, transform: newTransform);
        } else {
          return PlatformViewPosition(offset: outerOffset, transform: innerTransform);
        }
      }
    } else {
      if (innerOffset != null) {
        if (outerTransform != null) {
          final Matrix4 newTransform = Matrix4.translationValues(innerOffset.dx, innerOffset.dy, 0);
          newTransform.multiply(outerTransform);
          return PlatformViewPosition(offset: outerOffset, transform: newTransform);
        } else {
          final ui.Offset finalOffset = outerOffset != null ? (innerOffset + outerOffset) : innerOffset;
          return PlatformViewPosition(offset: finalOffset);
        }
      } else {
        return outer;
      }
    }
  }
}

class PlatformViewClip {
  const PlatformViewClip();
}

class PlatformViewStyling {
  const PlatformViewStyling({
    this.position = const PlatformViewPosition.zero(),
    this.clip = const PlatformViewClip(),
    this.opacity = 1.0
  });

  final PlatformViewPosition position;
  final PlatformViewClip clip;
  final double opacity;

  static PlatformViewStyling combine(PlatformViewStyling outer, PlatformViewStyling inner) {
    return PlatformViewStyling(
      position: PlatformViewPosition.combine(outer.position, inner.position),
    );
  }
}

class LayerBuilder {
  factory LayerBuilder.rootLayer() {
    return LayerBuilder._(null, RootLayer(), null);
  }

  factory LayerBuilder.childLayer({
    required LayerBuilder parent,
    required PictureLayer layer,
    required LayerOperation operation
  }) {
    return LayerBuilder._(parent, layer, operation);
  }

  LayerBuilder._(
    this.parent,
    this.layer,
    this.operation);

  final LayerBuilder? parent;
  final PictureLayer layer;
  final LayerOperation? operation;
  final List<PictureDrawCommand> pendingPictures = <PictureDrawCommand>[];
  List<PlatformView> pendingPlatformViews = <PlatformView>[];
  ui.Rect? picturesRect;
  ui.Rect? platformViewRect;

  PlatformViewStyling? _memoizedPlatformViewStyling;

  PlatformViewStyling _createPlatformViewStyling() {
    final PlatformViewStyling? innerStyling = operation?.createPlatformViewStyling();
    final PlatformViewStyling? outerStyling = parent?.platformViewStyling;
    if (innerStyling == null) {
      return outerStyling ?? const PlatformViewStyling();
    }
    if (outerStyling == null) {
      return innerStyling;
    }
    return PlatformViewStyling.combine(outerStyling, innerStyling);
  }

  PlatformViewStyling get platformViewStyling {
    return _memoizedPlatformViewStyling ??= _createPlatformViewStyling();
  }

  void flushSlices() {
    if (pendingPictures.isNotEmpty) {
      final ui.Rect drawnRect = picturesRect ?? ui.Rect.zero;
      final ui.Rect rect = operation?.cullRect(drawnRect) ?? drawnRect;
      final ui.PictureRecorder recorder = ui.PictureRecorder();
      final SceneCanvas canvas = ui.Canvas(recorder, rect) as SceneCanvas;

      operation?.pre(canvas, rect);
      for (final PictureDrawCommand command in pendingPictures) {
        if (command.offset != ui.Offset.zero) {
          canvas.save();
          canvas.translate(command.offset.dx, command.offset.dy);
          canvas.drawPicture(command.picture);
          canvas.restore();
        } else {
          canvas.drawPicture(command.picture);
        }
      }
      operation?.post(canvas, rect);
      final ui.Picture picture = recorder.endRecording();
      layer.slices.add(PictureSlice(picture as ScenePicture));
    }

    if (pendingPlatformViews.isNotEmpty) {
      ui.Rect? occlusionRect = platformViewRect;
      if (occlusionRect != null && operation != null) {
        occlusionRect = operation!.inverseMapRect(occlusionRect);
      }
      layer.slices.add(PlatformViewSlice(pendingPlatformViews, occlusionRect));
    }

    pendingPictures.clear();
    pendingPlatformViews = <PlatformView>[];
    picturesRect = null;
    platformViewRect = null;
  }

  void addPicture(
    ui.Offset offset,
    ui.Picture picture, {
    bool isComplexHint = false,
    bool willChangeHint = false
  }) {
    final ui.Rect cullRect = (picture as ScenePicture).cullRect;
    final ui.Rect shiftedRect = cullRect.shift(offset);
    if (platformViewRect?.overlaps(shiftedRect) ?? false) {
      flushSlices();
    }
    pendingPictures.add(PictureDrawCommand(offset, picture));
    picturesRect = picturesRect?.expandToInclude(shiftedRect) ?? shiftedRect;
  }

  void addPlatformView(
    int viewId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0
  }) {
    final ui.Rect bounds = ui.Rect.fromLTWH(offset.dx, offset.dy, width, height);
    platformViewRect = platformViewRect?.expandToInclude(bounds) ?? bounds;
    final PlatformViewStyling layerStyling = platformViewStyling;
    final PlatformViewStyling viewStyling = offset == ui.Offset.zero
      ? layerStyling
      : PlatformViewStyling.combine(
        PlatformViewStyling(
          position: PlatformViewPosition(offset: offset),
        ),
        layerStyling,
      );
    pendingPlatformViews.add(PlatformView(viewId, ui.Size(width, height), viewStyling));
  }

  void mergeLayer(PictureLayer layer) {
    for (final LayerSlice slice in layer.slices) {
      switch (slice) {
        case PictureSlice():
          addPicture(ui.Offset.zero, slice.picture);
        case PlatformViewSlice():
          final ui.Rect? occlusionRect = slice.occlusionRect;
          if (occlusionRect != null) {
            platformViewRect = platformViewRect?.expandToInclude(occlusionRect) ?? occlusionRect;
          }
          pendingPlatformViews.addAll(slice.views);
      }
    }
  }

  PictureLayer build() {
    flushSlices();
    return layer;
  }
}
