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
    // We need to do something similar in [HtmlViewEmbedder._compositeWithParams],
    // because tests often short-circuit the lifecycle of a Platform View, and
    // getSlot will throw an unwanted assertion.
    // Instead, we return a non-functional PlatformView, as the previous iteration
    // of the code did, so old (framework) tests keep passing.
    // See: https://github.com/flutter/engine/blob/ee1696721b02539d5f32b1cbfdf6bfa107663e91/lib/web_ui/lib/src/engine/html/platform_view.dart#L49-L55
    if (assertionsEnabled && !platformViewManager.knowsViewId(viewId)) {
      return html.DivElement()..id = 'only-to-vw-flutter/test/widgets/html_element_view_test.dart';
    }

    return platformViewManager.getSlot(viewId);
  }

  @override
  Matrix4? get localTransformInverse => null;

  @override
  void apply() {
    // See `_compositeWithParams` in the HtmlViewEmbedder for the canvaskit equivalent.
    rootElement!.style
      ..transform = 'translate(${dx}px, ${dy}px)'
      ..width = '${width}px'
      ..height = '${height}px'
      ..position = 'absolute';
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
