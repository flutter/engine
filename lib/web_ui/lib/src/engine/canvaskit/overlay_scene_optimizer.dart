// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

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
}

sealed class RenderingEntity {}

class RenderingRenderCanvas extends RenderingEntity {
  RenderingRenderCanvas();
  final List<CkPicture> pictures = <CkPicture>[];

  void add(CkPicture picture) {
    pictures.add(picture);
  }
}

class RenderingPlatformView extends RenderingEntity {
  RenderingPlatformView(this.viewId);
  final int viewId;
}

ui.Rect _computePlatformViewBounds(EmbeddedViewParams params) {
  return ui.Rect.largest;
}

Rendering createOptimizedRendering(
    List<CkPicture> pictures, List<int> platformViews) {
  assert(pictures.length == platformViews.length + 1);
  final Rendering result = Rendering();
  RenderingRenderCanvas currentRenderCanvas = RenderingRenderCanvas();
  currentRenderCanvas.add(pictures[0]);
  for (int i = 0; i < platformViews.length; i++) {}
  return result;
}
