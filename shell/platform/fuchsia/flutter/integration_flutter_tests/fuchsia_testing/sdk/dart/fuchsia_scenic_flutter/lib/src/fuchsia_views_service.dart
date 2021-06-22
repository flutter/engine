// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'dart:ui';

import 'package:flutter/services.dart';
import 'package:meta/meta.dart';

/// Defines a singleton [PlatformViewChannel] used to create and control
/// Fuchsia specific platform views.
class FuchsiaViewsService {
  // FuchsiaViewsService is a singleton because there is only ever one
  // entry-point into the native code for a given platform channel.
  static final FuchsiaViewsService instance = FuchsiaViewsService._();

  // The platform view channel used to communicate with flutter engine.
  final _platformViewChannel = MethodChannel(
    'flutter/platform_views',
    JSONMethodCodec(),
  );

  /// The [MethodChannel] used to communicate with Flutter Embedder.
  @visibleForTesting
  MethodChannel get platformViewChannel => _platformViewChannel;

  /// Holds the method call handlers registered by the view id.
  final _callHandlers = <int, Future<dynamic> Function(MethodCall call)?>{};

  // Private constructor. Registers a method call handler with the platform
  // view.
  FuchsiaViewsService._() {
    platformViewChannel.setMethodCallHandler((call) async {
      if (_callHandlers.isEmpty) {
        return;
      }

      // Guard against invalid or missing arguments.
      try {
        // Call the method call handler registered for viewId.
        int? viewId = call.arguments['viewId'];
        return _callHandlers[viewId]?.call(call);
        // ignore: avoid_catches_without_on_clauses
      } catch (e) {
        // If viewId is missing, call the last registered handler.
        return _callHandlers.values.last?.call(call);
      }
    });
  }

  /// Register a [MethodCall] handler for a given [viewId].
  void register(
          int viewId, Future<dynamic> Function(MethodCall call)? handler) =>
      _callHandlers[viewId] = handler;

  /// Deregister existing [MethodCall] handler for a given [viewId].
  void deregister(int viewId) => _callHandlers.remove(viewId);

  /// Creates a platform view with [viewId] and given properties.
  Future<void> createView(
    int viewId, {
    bool hitTestable = true,
    bool focusable = true,
    Rect viewOcclusionHint = Rect.zero,
  }) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'viewId': viewId,
      'hitTestable': hitTestable,
      'focusable': focusable,
      'viewOcclusionHintLTRB': <double>[
        viewOcclusionHint.left,
        viewOcclusionHint.top,
        viewOcclusionHint.right,
        viewOcclusionHint.bottom
      ],
    };
    return platformViewChannel.invokeMethod('View.create', args);
  }

  /// Updates view properties of the platform view associated with [viewId].
  Future<void> updateView(
    int viewId, {
    bool hitTestable = true,
    bool focusable = true,
    Rect viewOcclusionHint = Rect.zero,
  }) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'viewId': viewId,
      'hitTestable': hitTestable,
      'focusable': focusable,
      'viewOcclusionHintLTRB': <double>[
        viewOcclusionHint.left,
        viewOcclusionHint.top,
        viewOcclusionHint.right,
        viewOcclusionHint.bottom
      ],
    };
    return platformViewChannel.invokeMethod('View.update', args);
  }

  /// Destroys the platform view associated with [viewId].
  Future<void> destroyView(int viewId) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'viewId': viewId,
    };
    return platformViewChannel.invokeMethod('View.dispose', args);
  }

  /// Requests that focus be transferred to the remote Scene represented by
  /// this connection.
  Future<void> requestFocus(int viewRef) async {
    final args = <String, dynamic>{
      'viewRef': viewRef,
    };

    final result =
        await platformViewChannel.invokeMethod('View.requestFocus', args);
    // Throw OSError if result is non-zero.
    if (result != 0) {
      throw OSError(
        'Failed to request focus for view: $viewRef with $result',
        result,
      );
    }
  }
}
