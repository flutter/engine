// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

@JS('window.flutter_canvas_kit')
external CanvasKit get canvasKit2;

@JS()
class CanvasKit {
  @JS('BlendMode')
  external CanvasKitBlendMode get blendMode;
}

@JS()
class CanvasKitBlendMode {
  @JS('Clear')
  external int get clear;

  @JS('Src')
  external int get src;

  @JS('Dst')
  external int get dst;

  @JS('SrcOver')
  external int get srcOver;
}
