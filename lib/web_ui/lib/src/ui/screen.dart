// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9
part of ui;

/// A class representing the screen that application windows are displayed on.
abstract class Screen {
  /// A const constructor to allow subclasses to be const.
  const Screen();

  /// The value to use to indicate an invalid value for a [Screen] object.
  static const Screen invalid = engine.EngineScreen.invalid;

  /// The configuration of this screen.
  ScreenConfiguration get configuration;
}
