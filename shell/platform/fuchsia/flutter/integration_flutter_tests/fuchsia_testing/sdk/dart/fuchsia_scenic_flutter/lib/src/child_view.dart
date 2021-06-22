// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_as

import 'package:flutter/widgets.dart';
import 'package:meta/meta.dart';

import 'child_view_connection.dart';
import 'child_view_render_box.dart';
import 'child_view_render_box_2.dart';

/// A widget that is replaced by content from another process.
///
/// Requires a [MediaQuery] ancestor to provide appropriate media information to
/// the child.
@immutable
class ChildView extends LeafRenderObjectWidget {
  /// A connection to the child whose content will replace this widget.
  final ChildViewConnection? connection;

  /// Whether this child should be included during hit testing.
  ///
  /// Defaults to true.
  final bool hitTestable;

  /// Whether this child and its children should be allowed to receive focus.
  ///
  /// Defaults to true.
  final bool focusable;

  /// Creates a widget that is replaced by content from another process.
  ChildView({this.connection, this.hitTestable = true, this.focusable = true})
      : super(key: GlobalObjectKey(connection as Object));

  @override
  RenderBox createRenderObject(BuildContext context) {
    if (connection?.usePlatformView == true) {
      return ChildViewRenderBox2(
        connection: connection,
        hitTestable: hitTestable,
        focusable: focusable,
      );
    } else {
      return ChildViewRenderBox(
        connection: connection,
        hitTestable: hitTestable,
        focusable: focusable,
      );
    }
  }

  @override
  void updateRenderObject(BuildContext context, RenderObject renderObject) {
    assert(renderObject is ChildViewRenderBox ||
        renderObject is ChildViewRenderBox2);
    if (renderObject is ChildViewRenderBox) {
      renderObject
        ..connection = connection
        ..hitTestable = hitTestable
        ..focusable = focusable;
    } else if (renderObject is ChildViewRenderBox2) {
      renderObject
        ..connection = connection
        ..hitTestable = hitTestable
        ..focusable = focusable;
    }
  }
}
