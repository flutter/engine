// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'canvaskit/renderer.dart';
import 'dom.dart';
import 'safe_browser_api.dart';

/// Manages the browser's context menu for the entire app.
///
/// The context menu is the menu that appears on right click, for example.
class ContextMenu {
  const ContextMenu._();

  static bool _contextMenuEnabled = true;

  /// Handler for contextmenu events that prevents the browser's context menu
  /// from being shown.
  static final DomEventListener _handleContextMenu = allowInterop((DomEvent event) {
    event.preventDefault();
  });

  /// Disables the browser's context menu for the Flutter app.
  ///
  /// By default, when a Flutter web app starts, the context menu is enabled.
  ///
  /// Can be re-enabled by calling [enableContextMenu].
  static void disableContextMenu() {
    if (!_contextMenuEnabled) {
      return;
    }

    if (CanvasKitRenderer.instance.sceneHost != null) {
      CanvasKitRenderer.instance.sceneHost!.addEventListener('contextmenu', _handleContextMenu);
    } else {
      domWindow.addEventListener('contextmenu', _handleContextMenu);
    }
    _contextMenuEnabled = false;
  }

  /// Enables the browser's context menu for the Flutter app.
  ///
  /// By default, when a Flutter web app starts, the context menu is already
  /// enabled. Typically, this method would be used after calling
  /// [disableContextMenu] to first disable it.
  static void enableContextMenu() {
    if (_contextMenuEnabled) {
      return;
    }

    domWindow.removeEventListener('contextmenu', _handleContextMenu);
    _contextMenuEnabled = true;
  }
}
