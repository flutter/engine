// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';
import 'scene.dart';

/// Composits [Scenelet] objects into a given [hostElement].
///
/// A compositor is a stateful long-lived object. One compositor should be
/// created for a given [FlutterView] and it should match that view's lifespan.
/// This object holds references to DOM elements that comprise the rendering of
/// of the view.
class SceneletCompositor {
  SceneletCompositor(this.hostElement);

  final DomElement hostElement;

  void render(SceneletScene scene) {
    // TODO: Implement the following steps
    //       * "Sink" invisible platform views to reduce the total number of
    //         overlays needed to render the scene.
    //       * Group render scenelets and platform view scenelets into scenelet
    //         overlays.
    //       * Diff the new overlay list with the old overlay list and figure
    //         out what, if anything needs to be added, moved, and removed.
    //       * Tell the overlays to render themselves.
  }
}
