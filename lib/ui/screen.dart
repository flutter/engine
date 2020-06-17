// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9
part of dart.ui;

/// A class representing the screen that application windows are displayed on.
///
/// Each screen can have separate dimensions, and a separate device pixel ratio.
class Screen {
  const Screen._(Object screenId, PlatformDispatcher? platformDispatcher)
    : _screenId = screenId,
      _platformDispatcher = platformDispatcher;

  /// The opaque ID for this screen.
  final Object _screenId;

  /// The value to use to indicate an invalid value for a [Screen] object.
  static const Screen invalid = Screen._(-1, null);

  /// The platform dispatcher that this screen is registered with.
  ///
  /// Will only be null if the [Screen] is invalid.
  final PlatformDispatcher? _platformDispatcher;

  /// The configuration details of this screen.
  ScreenConfiguration get configuration {
    assert(_platformDispatcher?._screens.containsKey(_screenId) ?? false);
    return _platformDispatcher?._screenConfigurations[_screenId] ?? const ScreenConfiguration();
  }
}
