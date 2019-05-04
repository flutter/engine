// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
class Scene {
  /// Stub implementation. See docs in `../ui/`.
  Scene._();


  /// Stub implementation. See docs in `../ui/`.
  Future<Image> toImage(int width, int height) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void dispose() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class SceneBuilder {
  SceneBuilder();

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushTransform(Float64List matrix4) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushOffset(double dx, double dy) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushClipRect(Rect rect, {Clip clipBehavior = Clip.antiAlias}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushClipRRect(RRect rrect, {Clip clipBehavior = Clip.antiAlias}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushClipPath(Path path, {Clip clipBehavior = Clip.antiAlias}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushOpacity(int alpha, {Offset offset = Offset.zero}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushColorFilter(Color color, BlendMode blendMode) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushBackdropFilter(ImageFilter filter) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer pushShaderMask(Shader shader, Rect maskRect, BlendMode blendMode) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  // ignore: deprecated_member_use
  EngineLayer pushPhysicalShape({ Path path, double elevation, Color color, Color shadowColor, Clip clipBehavior = Clip.none}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void pop() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  EngineLayer addRetained(EngineLayer retainedLayer) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  // Values above must match constants in //engine/src/sky/compositor/performance_overlay_layer.h
  void addPerformanceOverlay(int enabledOptions, Rect bounds) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addPicture(Offset offset, Picture picture, { bool isComplexHint: false, bool willChangeHint: false }) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addTexture(int textureId, { Offset offset: Offset.zero, double width: 0.0, double height: 0.0 , bool freeze: false}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addPlatformView(int viewId, { Offset offset: Offset.zero, double width: 0.0, double height: 0.0}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addChildScene({
    Offset offset: Offset.zero,
    double width: 0.0,
    double height: 0.0,
    SceneHost sceneHost,
    bool hitTestable: true
  }) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void setRasterizerTracingThreshold(int frameInterval) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void setCheckerboardRasterCacheImages(bool checkerboard) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void setCheckerboardOffscreenLayers(bool checkerboard) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Scene build() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class SceneHost {
  /// Stub implementation. See docs in `../ui/`.
  SceneHost(dynamic exportTokenHandle);
  SceneHost.fromViewHolderToken(
      dynamic viewHolderTokenHandle,
      void Function() viewConnectedCallback,
      void Function() viewDisconnectedCallback,
      void Function(bool) viewStateChangedCallback);

  /// Stub implementation. See docs in `../ui/`.
  void dispose() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void setProperties(
      double width,
      double height,
      double insetTop,
      double insetRight,
      double insetBottom,
      double insetLeft,
      bool focusable) {
        throw UnimplementedError();
      }
}
