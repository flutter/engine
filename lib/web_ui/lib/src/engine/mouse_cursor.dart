// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


part of engine;

/// Provides mouse cursor bindings, such as the `flutter/mousecursor` channel.
class MouseCursor {
  /// Initializes the [MouseCursor] singleton.
  ///
  /// Use the [instance] getter to get the singleton after calling this method.
  static void initialize() {
    _instance ??= MouseCursor._();
  }

  /// The [MouseCursor] singleton.
  static MouseCursor? get instance => _instance;
  static MouseCursor? _instance;

  MouseCursor._() {}

  // Map from Flutter's kind values to CSS's cursor values.
  //
  // This map must be kept in sync with flutter's rendering/mouse_cursor.dart.
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
    'horizontalDoubleArrow': 'ew-resize',
    'move': 'move',
    'none': 'none',
    'noDrop': 'no-drop',
    'precise': 'crosshair',
    'progress': 'progress',
    'text': 'text',
    'upLeftDoubleArrow': 'nwse-resize',
    'upRightDoubleArrow': 'nesw-resize',
    'verticalDoubleArrow': 'ns-resize',
    'verticalText': 'vertical-text',
    'wait': 'wait',
    'zoomIn': 'zoom-in',
    'zoomOut': 'zoom-out',
  };
  static String _mapKindToCssValue(String? kind) {
    return _kindToCssValueMap[kind] ?? 'default';
  }

  void activateSystemCursor(String? kind) {
    domRenderer.setElementStyle(
      domRenderer.glassPaneElement!,
      'cursor',
      _mapKindToCssValue(kind),
    );
  }
}
