// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: unnecessary_null_comparison

import 'dart:ui';
import 'package:flutter/rendering.dart';

import 'child_view_connection.dart';

// ignore_for_file: public_member_api_docs

/// A |RenderBox| that allows hit-testing and focusing of a |ChildViewConnection|.  Renders itself by creating a |PlatformViewLayer|.
class ChildViewRenderBox2 extends RenderBox {
  ChildViewConnection? _connection;

  bool? _hitTestable;
  bool? _focusable;
  bool _connected = false;

  /// Creates a |RenderBox| for the child view.
  ChildViewRenderBox2({
    ChildViewConnection? connection,
    bool hitTestable = true,
    bool focusable = true,
  })  : assert(hitTestable != null),
        assert(focusable != null) {
    connection
        ?.connect(hitTestable: hitTestable, focusable: focusable)
        .then((_) {
      _connected = true;
      _connection = connection;
      _hitTestable = hitTestable;
      _focusable = focusable;
      markNeedsLayout();
      markNeedsPaint();
    });
  }

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
  bool get focusable => _focusable!;

  set focusable(bool value) {
    assert(value != null);
    if (value == _focusable) {
      return;
    }
    _focusable = value;
    if (_connection != null) {
      _connection!
          .setViewProperties(focusable: _focusable, hitTestable: _hitTestable);
      markNeedsLayout();
    }
  }

  /// Whether this child should be included during hit testing.
  bool get hitTestable => _hitTestable!;

  set hitTestable(bool value) {
    assert(value != null);
    if (value == _hitTestable) {
      return;
    }
    _hitTestable = value;
    if (_connection != null) {
      _connection!
          .setViewProperties(focusable: _focusable, hitTestable: _hitTestable);
      markNeedsPaint();
    }
  }

  @override
  bool get alwaysNeedsCompositing => true;

  @override
  bool get isRepaintBoundary => true;

  @override
  bool hitTestSelf(Offset position) => true;

  @override
  void performLayout() {
    size = constraints.biggest;
  }

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
    if (!_connected ||
        _connection?.viewId == null ||
        _connection!.viewId == 0) {
      return;
    }

    context.addLayer(
        PlatformViewLayer(rect: offset & size, viewId: _connection!.viewId));
  }
}
