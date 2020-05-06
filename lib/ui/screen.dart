// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of dart.ui;

/// A class representing the screen that application windows are displayed on.
class Screen {
    Screen._({Object screenId, PlatformDispatcher platformDispatcher})
    : _screenId = screenId,
      _platformDispatcher = platformDispatcher;

  /// The opaque ID for this screen.
  final Object _screenId;

  /// The platform dispatcher that this screen is registered with.
  final PlatformDispatcher _platformDispatcher;

  /// The configuration of this screen.
  ScreenConfiguration get configuration {
    assert(_platformDispatcher._screens.containsKey(_screenId));
    return _platformDispatcher._screenConfigurations[_screenId];
  }
}