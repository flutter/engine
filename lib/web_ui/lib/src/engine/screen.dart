// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

/// A class representing the screen that application windows are displayed on.
class EngineScreen extends ui.Screen {
  EngineScreen({Object screenId, ui.PlatformDispatcher platformDispatcher})
      : _screenId = screenId,
        _platformDispatcher = platformDispatcher;

  /// The opaque ID for this screen.
  final Object _screenId;

  /// The platform dispatcher that this screen is registered with.
  final ui.PlatformDispatcher _platformDispatcher;

  /// The configuration of this screen.
  ui.ScreenConfiguration get configuration {
    final EnginePlatformDispatcher engineDispatcher = _platformDispatcher as EnginePlatformDispatcher;
    assert(engineDispatcher._screenConfigurations.containsKey(_screenId));
    return engineDispatcher._screenConfigurations[_screenId];
  }
}
