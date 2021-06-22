// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: override_on_non_overriding_member, unnecessary_null_comparison

import 'dart:ui';
import 'package:flutter/rendering.dart';

import 'child_view_connection.dart';

/// A |RenderBox| that allows hit-testing and focusing of a |ChildViewConnection|.  Renders itself by creating a |ChildSceneLayer|.
class ChildViewRenderBox extends RenderBox {
  ChildViewConnection? _connection;

  bool _hitTestable;
  bool _focusable;

  double? _width;
  double? _height;

  /// Creates a |RenderBox| for the child view.
  ChildViewRenderBox({
    ChildViewConnection? connection,
    bool hitTestable = true,
    bool focusable = true,
  })  : _connection = connection,
        _hitTestable = hitTestable,
        _focusable = focusable,
        assert(hitTestable != null);

  /// The child to display.
  ChildViewConnection? get connection => _connection;
  set connection(ChildViewConnection? value) {
    if (value == _connection) {
      return;
    }
    _connection = value;
    if (_connection != null) {
      markNeedsLayout();
    }
    markNeedsPaint();
  }

  /// Whether this child should be able to recieve focus events
  bool get focusable => _focusable;

  set focusable(bool value) {
    assert(value != null);
    if (value == _focusable) {
      return;
    }
    _focusable = value;
    if (_connection != null) {
      markNeedsLayout();
    }
  }

  /// Whether this child should be included during hit testing.
  bool get hitTestable => _hitTestable;

  set hitTestable(bool value) {
    assert(value != null);
    if (value == _hitTestable) {
      return;
    }
    _hitTestable = value;
    if (_connection != null) {
      markNeedsPaint();
    }
  }

  @override
  bool get alwaysNeedsCompositing => true;

  @override
  bool hitTestSelf(Offset position) => true;

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder description) {
    super.debugFillProperties(description);
    description.add(
      DiagnosticsProperty<ChildViewConnection>(
        'connection',
        connection,
      ),
    );
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    // Ignore if we have no child view connection.
    assert(needsCompositing);
    if (_connection?.sceneHost == null) {
      return;
    }

    context.addLayer(_ChildSceneLayer(
      offset: offset,
      width: _width,
      height: _height,
      sceneHost: _connection!.sceneHost,
      hitTestable: _hitTestable,
    ));
  }

  @override
  void performLayout() {
    size = constraints.biggest;

    // Ignore if we have no child view connection.
    if (_connection == null) {
      return;
    }

    if (_width != null && _height != null) {
      double deltaWidth = (_width! - size.width).abs();
      double deltaHeight = (_height! - size.height).abs();

      // Ignore insignificant changes in size that are likely rounding errors.
      if (deltaWidth < 0.0001 && deltaHeight < 0.0001) {
        return;
      }
    }

    _width = size.width;
    _height = size.height;
    _connection!.setChildProperties(_width!, _height!, 0.0, 0.0, 0.0, 0.0,
        focusable: _focusable);
  }
}

/// A layer that represents content from another process.
class _ChildSceneLayer extends Layer {
  /// Creates a layer that displays content rendered by another process.
  ///
  /// All of the arguments must not be null.
  _ChildSceneLayer({
    this.offset = Offset.zero,
    this.width = 0.0,
    this.height = 0.0,
    this.sceneHost,
    this.hitTestable = true,
  });

  /// Offset from parent in the parent's coordinate system.
  Offset offset;

  /// The horizontal extent of the child, in logical pixels.
  double? width;

  /// The vertical extent of the child, in logical pixels.
  double? height;

  /// The host site for content rendered by the child.
  SceneHost? sceneHost;

  /// Whether this child should be included during hit testing.
  ///
  /// Defaults to true.
  bool hitTestable;

  @override
  EngineLayer? addToScene(SceneBuilder builder,
      [Offset layerOffset = Offset.zero]) {
    assert(sceneHost != null);

    builder.addChildScene(
      offset: offset + layerOffset,
      width: width!,
      height: height!,
      sceneHost: sceneHost!,
      hitTestable: hitTestable,
    );
    return null;
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder description) {
    super.debugFillProperties(description);
    description
      ..add(DiagnosticsProperty<Offset>('offset', offset))
      ..add(DoubleProperty('width', width))
      ..add(DoubleProperty('height', height))
      ..add(DiagnosticsProperty<SceneHost>('sceneHost', sceneHost))
      ..add(DiagnosticsProperty<bool>('hitTestable', hitTestable));
  }

  @override
  S? find<S extends Object>(Offset regionOffset) => null;

  @override
  Iterable<S> findAll<S>(Offset regionOffset) {
    return <S>[];
  }

  @override
  bool findAnnotations<S extends Object>(
          AnnotationResult<S> result, Offset localPosition,
          {bool? onlyFirst}) =>
      false;

  @override
  AnnotationResult<S> findAllAnnotations<S extends Object>(
          Offset localPosition) =>
      AnnotationResult<S>();
}
