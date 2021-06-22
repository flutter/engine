// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_as, unnecessary_null_comparison

import 'dart:async';
import 'dart:ui';

import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import 'fuchsia_views_service.dart';

/// Defines a callback to receive view connected and disconnected event.
typedef FuchsiaViewConnectionCallback = void Function(
    FuchsiaViewController connection);

/// Defines a callback to receive view state changed event.
typedef FuchsiaViewConnectionStateCallback = void Function(
    FuchsiaViewController connection, bool? newState);

/// Defines a callback to receive pointer events for the embedded view.
typedef FuchsiaPointerEventsCallback = Future<void> Function(
    FuchsiaViewController, PointerEvent);

/// A connection to a fuchsia view.  It can be used to construct a [FuchsiaView]
/// widget that will display the view's contents on their own layer.
class FuchsiaViewController implements PlatformViewController {
  /// The raw value of the [ViewHolderToken] where this view is attached.
  @override
  final int viewId;

  /// Callback when the connection to child's view is connected to view tree.
  final FuchsiaViewConnectionCallback? onViewConnected;

  /// Callback when the child's view is disconnected from view tree.
  final FuchsiaViewConnectionCallback? onViewDisconnected;

  /// Callback when the child view's state changes.
  final FuchsiaViewConnectionStateCallback? onViewStateChanged;

  /// Callback when pointer events are dispatched on top of child view.
  final FuchsiaPointerEventsCallback? onPointerEvent;

  /// Returns the [FuchsiaViewsService] used to communicated with the embedder.
  @visibleForTesting
  FuchsiaViewsService get fuchsiaViewsService => FuchsiaViewsService.instance;

  // The [Completer] that is completed when the platform view is connected.
  var _whenConnected = Completer();

  /// The future that completes when the platform view is connected.
  Future get whenConnected => _whenConnected.future;

  /// Returns true when platform view is connected.
  bool get connected => _whenConnected.isCompleted;

  /// Constructor.
  FuchsiaViewController({
    required this.viewId,
    this.onViewConnected,
    this.onViewDisconnected,
    this.onViewStateChanged,
    this.onPointerEvent,
  }) : assert(viewId != null);

  /// Connects to the platform view given it's [viewId].
  ///
  /// Called by [FuchsiaView] when the platform view is ready to be initialized
  /// and should not be called directly.
  Future<void> connect({
    bool hitTestable = true,
    bool focusable = true,
    Rect viewOcclusionHint = Rect.zero,
  }) async {
    if (_whenConnected.isCompleted) return;

    // Setup callbacks for receiving view events.
    fuchsiaViewsService.register(viewId, (call) async {
      switch (call.method) {
        case 'View.viewConnected':
          _whenConnected.complete();
          onViewConnected?.call(this);
          break;
        case 'View.viewDisconnected':
          _whenConnected = Completer();
          onViewDisconnected?.call(this);
          break;
        case 'View.viewStateChanged':
          final state =
              call.arguments['state'] == 1 || call.arguments['state'] == true;
          onViewStateChanged?.call(this, state);
          break;
        default:
          print('Unknown method call from platform view channel: $call');
      }
    });

    // Now send a create message to the platform view.
    return fuchsiaViewsService.createView(
      viewId,
      hitTestable: hitTestable,
      focusable: focusable,
      viewOcclusionHint: viewOcclusionHint,
    );
  }

  /// Disconnects the view from the [ViewHolderToken].
  ///
  /// This should not be called in [onViewDisconnected] callback. The need to
  /// disconnect, without exiting the underlying component is rare. Most views
  /// are closed by first exiting their component, in which case the callback
  /// [onViewDisconnect] is invoked.
  Future<void> disconnect() async {
    fuchsiaViewsService.deregister(viewId);
    await fuchsiaViewsService.destroyView(viewId);
    onViewDisconnected?.call(this);
  }

  /// Updates properties on the platform view given it's [viewId].
  ///
  /// Called by [FuchsiaView] when the [focusable] or [hitTestable] or
  /// [viewOcclusionHint] properties are changed.
  Future<void> update({
    bool focusable = true,
    bool hitTestable = true,
    Rect viewOcclusionHint = Rect.zero,
  }) async {
    return fuchsiaViewsService.updateView(
      viewId,
      hitTestable: hitTestable,
      focusable: focusable,
      viewOcclusionHint: viewOcclusionHint,
    );
  }

  /// Requests that focus be transferred to the remote Scene represented by
  /// this connection.
  Future<void> requestFocus(int viewRef) async {
    return fuchsiaViewsService.requestFocus(viewRef);
  }

  /// Dispose the underlying platform view controller.
  @override
  Future<void> dispose() async {}

  @override
  Future<void> clearFocus() async {}

  /// Dispatch pointer events for the child view.
  @override
  Future<void> dispatchPointerEvent(PointerEvent event) async {
    return onPointerEvent?.call(this, event);
  }

  /// Returns the viewport rect of the child view in parent's coordinates.
  ///
  /// This only works if there is one and only one widget that has the [Key]
  /// set to [GlobalObjectKey] using this constroller's instance. [FuchsiaView]
  /// set's its [key] to the controller instance passed to it.
  ///
  /// Returns [null] if the child view has not been rendered.
  Rect? get viewport {
    final key = GlobalObjectKey(this);
    RenderBox? box = key.currentContext?.findRenderObject() as RenderBox?;
    if (box?.hasSize == true) {
      final offset = box!.localToGlobal(Offset.zero);
      return offset & box.size;
    }
    return null;
  }
}
