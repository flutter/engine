// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'dart:ui';
import 'package:flutter/services.dart';

import 'package:fidl_fuchsia_ui_views/fidl_async.dart';

// ignore_for_file: public_member_api_docs, unnecessary_null_comparison

typedef ChildViewConnectionCallback = void Function(
    ChildViewConnection connection);
typedef ChildViewConnectionStateCallback = void Function(
    ChildViewConnection connection, bool? newState);

/// A connection to a child view.  It can be used to construct a [ChildView]
/// widget that will display the view's contents on their own layer.
class ChildViewConnection {
  /// [ViewHolderToken] of the child view.
  final ViewHolderToken viewHolderToken;

  /// The optional [ViewRef] of the view.
  final ViewRef? viewRef;

  /// Callback when the connection to child's view is established.
  final ChildViewConnectionCallback? onAvailable;

  /// Callback when the child's view is disconnected.
  final ChildViewConnectionCallback? onUnavailable;

  /// Callback when the child view's state changes.
  final ChildViewConnectionStateCallback? onStateChanged;

  /// Whether to use [PlatformView] feature of Flutter.
  bool usePlatformView;

  /// SceneHost used to reference and render content from a remote Scene.
  SceneHost? get sceneHost => _sceneHost;
  SceneHost? _sceneHost;

  final _platformViewChannel = MethodChannel(
    'flutter/platform_views',
    JSONMethodCodec(),
  );

  /// Creates this connection from a ViewHolderToken.
  ChildViewConnection(
    this.viewHolderToken, {
    this.viewRef,
    this.onAvailable,
    this.onUnavailable,
    this.onStateChanged,
    this.usePlatformView = false,
  })  : assert(viewHolderToken.value != null && viewHolderToken.value.isValid),
        assert(
            viewRef?.reference == null || viewRef!.reference.handle!.isValid) {
    if (!usePlatformView) {
      _sceneHost = SceneHost(
          viewHolderToken.value.passHandle(),
          (onAvailable == null)
              ? null
              : () {
                  onAvailable!(this);
                },
          (onUnavailable == null)
              ? null
              : () {
                  onUnavailable!(this);
                },
          (onStateChanged == null)
              ? null
              : (bool state) {
                  onStateChanged!(this, state);
                });
    } else {
      try {
        _platformViewChannel.setMethodCallHandler((call) async {
          switch (call.method) {
            case 'View.viewConnected':
              onAvailable?.call(this);
              break;
            case 'View.viewDisconnected':
              onUnavailable?.call(this);
              break;
            case 'View.viewStateChanged':
              onStateChanged?.call(this, call.arguments['state'] ?? false);
              break;
            default:
              print('Unknown method call from platform view channel: $call');
          }
        });
      } on Exception catch (e) {
        print('Failed to set method call handler: $e');
      }
    }
  }

  /// Gets the view id from [viewHolderToken].
  int get viewId => viewHolderToken.value.handle!.handle;

  /// Releases native resources held by [SceneHost] object in Flutter engine.
  void dispose() {
    if (usePlatformView) {
      final args = <String, dynamic>{'viewId': viewId};
      _platformViewChannel.invokeMethod('View.dispose', args);
    } else {
      _sceneHost!.dispose();
      _sceneHost = null;
    }
  }

  /// Called by [ChildViewRenderBox2] when the platform view is ready to
  /// be initialized.
  Future<void> connect({bool? hitTestable, bool? focusable}) async {
    final args = <String, dynamic>{
      'viewId': viewId,
      'hitTestable': hitTestable,
      'focusable': focusable,
    };
    await _platformViewChannel.invokeMethod('View.create', args);
  }

  /// Requests that focus be transferred to the remote Scene represented by
  /// this connection.
  Future<void> requestFocus() async {
    assert(viewRef != null);
    final args = <String, dynamic>{
      'viewRef': viewRef!.reference.handle!.handle
    };
    final result =
        await _platformViewChannel.invokeMethod('View.requestFocus', args);
    // Throw OSError if result is non-zero.
    if (result != 0) {
      final koid = viewRef!.reference.handle!.koid;
      final error = Error(result);
      throw OSError(
        'Failed to request focus for view: $koid with $error',
        result,
      );
    }
  }

  /// Sets properties on the remote Scene represented by this connection.
  void setChildProperties(double width, double height, double insetTop,
      double insetRight, double insetBottom, double insetLeft,
      {bool focusable = true}) {
    _sceneHost?.setProperties(
        width, height, insetTop, insetRight, insetBottom, insetLeft, focusable);
  }

  /// Sets properties on the remote Scene represented by this connection.
  ///
  /// Called by [ChildViewRenderBox2] when the [focusable] and [hitTestable]
  /// properties are changed.
  void setViewProperties({bool? focusable = true, bool? hitTestable = true}) {
    final args = <String, dynamic>{
      'viewId': viewId,
      'hitTestable': hitTestable,
      'focusable': focusable,
    };
    _platformViewChannel.invokeMethod('View.update', args);
  }
}
