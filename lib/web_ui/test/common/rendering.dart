// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

// The scene that will be rendered in the next call to `onDrawFrame`.
ui.Scene? _sceneToRender;

/// Sets up rendering so that `onDrawFrame` will render the last requested
/// scene.
void setUpRenderingForTests() {
  // Set `onDrawFrame` to call `renderer.renderScene`.
  EnginePlatformDispatcher.instance.onDrawFrame = () {
    if (_sceneToRender != null) {
      EnginePlatformDispatcher.instance.render(_sceneToRender!);
      _sceneToRender = null;
    }
  };
}

/// Render the given [scene] in an `onDrawFrame` scope.
void renderScene(ui.Scene scene) {
  _sceneToRender = scene;
  EnginePlatformDispatcher.instance.invokeOnDrawFrame();
}
