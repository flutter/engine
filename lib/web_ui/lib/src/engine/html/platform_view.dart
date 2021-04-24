// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// A surface containing a platform view, which is an HTML element.
class PersistedPlatformView extends PersistedLeafSurface {
  final int viewId;
  final double dx;
  final double dy;
  final double width;
  final double height;

  PersistedPlatformView(this.viewId, this.dx, this.dy, this.width, this.height);

  @override
  html.Element createElement() {
    return platformViewManager.getSlot(viewId);
  }

  @override
  Matrix4? get localTransformInverse => null;

  // Applies the width/height information in the `content` element of the Platform View.
  //
  // See `_updateContentSize` in the HtmlViewEmbedder for the canvaskit version.
  void _applyOnContent() {
    final html.Element content = platformViewManager.getContent(viewId);
    content.style
      ..width = '${width}px'
      ..height = '${height}px'
      ..position = 'absolute';
  }

  @override
  void apply() {
    rootElement!.style
      ..transform = 'translate(${dx}px, ${dy}px)';

    _applyOnContent();
  }

  // Platform Views can only be updated if their viewId matches.
  @override
  bool canUpdateAsMatch(PersistedSurface oldSurface) {
    if (super.canUpdateAsMatch(oldSurface)) {
      // super checks the runtimeType of the surface, so we can just cast...
      return viewId == ((oldSurface as PersistedPlatformView).viewId);
    }
    return false;
  }

  @override
  double matchForUpdate(PersistedPlatformView existingSurface) {
    return existingSurface.viewId == viewId ? 0.0 : 1.0;
  }

  @override
  void update(PersistedPlatformView oldSurface) {
    assert(
      viewId == oldSurface.viewId,
      'PersistedPlatformView with different viewId should never be updated. Check the canUpdateAsMatch method.',
    );
    super.update(oldSurface);
    // Only update if the view has been resized
    if (dx != oldSurface.dx ||
        dy != oldSurface.dy ||
        width != oldSurface.width ||
        height != oldSurface.height) {
      // A change in any of the dimensions is performed by calling apply.
      apply();
    }
  }
}
