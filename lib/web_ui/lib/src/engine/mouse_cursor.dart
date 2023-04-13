// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'embedder.dart';
import 'util.dart';

/// Provides mouse cursor bindings, such as the `flutter/mousecursor` channel.
class MouseCursor {
  MouseCursor._();

  /// Initializes the [MouseCursor] singleton.
  ///
  /// Use the [instance] getter to get the singleton after calling this method.
  static void initialize() {
    _instance ??= MouseCursor._();
  }

  /// The [MouseCursor] singleton.
  static MouseCursor? get instance => _instance;
  static MouseCursor? _instance;

  // Map from Flutter's kind values to CSS's cursor values.
  //
  // This map must be kept in sync with Flutter framework's
  // rendering/mouse_cursor.dart.
  static const Map<String, String> _kindToCssValueMap = <String, String>{
    'alias': 'alias',
    'allScroll': 'all-scroll',
    'basic': 'default',
    'cell': 'cell',
    'click': 'pointer',
    'contextMenu': 'context-menu',
    'copy': 'copy',
    'forbidden': 'not-allowed',
    'grab': 'grab',
    'grabbing': 'grabbing',
    'help': 'help',
    'move': 'move',
    'none': 'none',
    'noDrop': 'no-drop',
    'precise': 'crosshair',
    'progress': 'progress',
    'text': 'text',
    'resizeColumn': 'col-resize',
    'resizeDown': 's-resize',
    'resizeDownLeft': 'sw-resize',
    'resizeDownRight': 'se-resize',
    'resizeLeft': 'w-resize',
    'resizeLeftRight': 'ew-resize',
    'resizeRight': 'e-resize',
    'resizeRow': 'row-resize',
    'resizeUp': 'n-resize',
    'resizeUpDown': 'ns-resize',
    'resizeUpLeft': 'nw-resize',
    'resizeUpRight': 'ne-resize',
    'resizeUpLeftDownRight': 'nwse-resize',
    'resizeUpRightDownLeft': 'nesw-resize',
    'verticalText': 'vertical-text',
    'wait': 'wait',
    'zoomIn': 'zoom-in',
    'zoomOut': 'zoom-out',
  };
  static String _mapKindToCssValue(String? kind) {
    // Allow 'image-set(...)'/'-webkit-image-set(...)' css commands thru for setting DevicePixelRatio
    // aware images for cursors (for newer browsers) and allow 'url(...)' strings thru as fallback
    // for older browsers (bare url() commands are always 1.0x dpr).  The hotspot coordinates are always
    // supplied in 1.0x dpr coordinates.  All of these css methods can use data-uri versions of url's
    // which allow for inline definition of image data to define the cursor.
    if (kind != null &&
        ((kind.startsWith('url') ||
            kind.startsWith('image-set') ||
            kind.startsWith('-webkit-image-set')))) {
      return kind;
    } else {
      return _kindToCssValueMap[kind] ?? 'default';
    }
  }

  void activateSystemCursor(String? kind) {
    setElementStyle(
      flutterViewEmbedder.glassPaneElement,
      'cursor',
      _mapKindToCssValue(kind),
    );
  }
}
