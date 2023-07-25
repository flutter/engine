// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';

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

    

    queuedRenders -= 1;
  }
}
