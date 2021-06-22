// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:flutter/foundation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'fuchsia_view_controller.dart';

/// A widget that is replaced by content from another process.
class FuchsiaView extends StatefulWidget {
  /// The [PlatformViewController] used to control this [FuchsiaView].
  final FuchsiaViewController controller;

  /// Whether this child should be included during hit testing.
  ///
  /// Defaults to true.
  final bool hitTestable;

  /// Whether this child and its children should be allowed to receive focus.
  ///
  /// Defaults to true.
  final bool focusable;

  /// View occlusion hint passed to the child view.
  ///
  /// Defaults to [Rect.zero].
  final Rect viewOcclusionHint;

  /// Creates a widget that is replaced by content from another process.
  FuchsiaView({
    required this.controller,
    this.hitTestable = true,
    this.focusable = true,
    this.viewOcclusionHint = Rect.zero,
  }) : super(key: GlobalObjectKey(controller));

  @override
  _FuchsiaViewState createState() => _FuchsiaViewState();
}

class _FuchsiaViewState extends State<FuchsiaView> {
  bool _needsUpdate = false;

  @override
  void initState() {
    super.initState();

    // Apply any pending updates once the platform view is connected.
    widget.controller.whenConnected.then((_) => _updateView());
  }

  @override
  void didUpdateWidget(FuchsiaView oldWidget) {
    super.didUpdateWidget(oldWidget);

    if (widget.focusable != oldWidget.focusable ||
        widget.hitTestable != oldWidget.hitTestable ||
        widget.viewOcclusionHint != oldWidget.viewOcclusionHint) {
      _needsUpdate = true;
      _updateView();
    }
  }

  // Updates the view attributes on the underlying platform view.
  //
  // Called when view's [focusable], [hitTestable] or [viewOcclusionHint] have
  // changed or when the underlying platform view is connected.
  void _updateView() {
    if (_needsUpdate == false || !widget.controller.connected) {
      return;
    }

    widget.controller.update(
        focusable: widget.focusable,
        hitTestable: widget.hitTestable,
        viewOcclusionHint: widget.viewOcclusionHint);
    _needsUpdate = false;
  }

  @override
  Widget build(BuildContext context) {
    return PlatformViewLink(
      viewType: 'fuchsiaView',
      onCreatePlatformView: (params) => widget.controller
        ..connect(
          hitTestable: widget.hitTestable,
          focusable: widget.focusable,
          viewOcclusionHint: widget.viewOcclusionHint,
        ).then((_) {
          params.onPlatformViewCreated(widget.controller.viewId);
        }),
      surfaceFactory: (context, controller) {
        return PlatformViewSurface(
          gestureRecognizers: const <Factory<OneSequenceGestureRecognizer>>{},
          controller: controller,
          hitTestBehavior: widget.hitTestable
              ? PlatformViewHitTestBehavior.opaque
              : PlatformViewHitTestBehavior.transparent,
        );
      },
    );
  }
}
