// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
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
  static MouseCursor get instance => _instance;
  static MouseCursor _instance;

  MouseCursor._() {
    registerHotRestartListener(() {
      dispose();
    });
  }

  static String _mapShapeCodeToCssValue(int shapeCode) {
    // Shape codes are hard-coded identifiers for system cursors.
    // 
    // The shape code values must be kept in sync with flutter's
    // rendering/mouse_cursor.dart
    switch (shapeCode) {
      case /* none */ 0x334c4a:
        return 'none';
      case /* basic */ 0xf17aaa:
        return 'default';
      case /* click */ 0xa8affc:
        return 'pointer';
      case /* text */ 0x1cb251:
        return 'text';
      case /* forbidden */ 0x350f9d:
        return 'not-allowed';
      case /* grab */ 0x28b91f:
        return 'grab';
      case /* grabbing */ 0x6631ce:
        return 'grabbing';
      default:
        return 'default';
    }
  }

  void activateSystemCursor(int shapeCode) {
    domRenderer.setElementStyle(
      domRenderer.glassPaneElement,
      'cursor',
      _mapShapeCodeToCssValue(shapeCode),
    );
  }

  /// Uninitializes the [MouseCursor] singleton.
  ///
  /// After calling this method this object becomes unusable and [instance]
  /// becomes `null`. Call [initialize] again to initialize a new singleton.
  void dispose() {
  }

  static const StandardMessageCodec _messageCodec = StandardMessageCodec();

}
