// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

// These are additional APIs that are not part of the `dart:ui` interface that
// are needed internally to properly implement a `SceneBuilder` on top of the
// generic Canvas/Picture api.
abstract class SceneCanvas implements ui.Canvas {
  void saveLayerWithFilter(ui.Rect? bounds, ui.Paint paint, ui.ImageFilter filter);
}

abstract class ScenePicture implements ui.Picture {
  ui.Rect get cullRect;
}

abstract class SceneImageFilter implements ui.ImageFilter {
  ui.Rect filterBounds(ui.Rect inputBounds);
}
