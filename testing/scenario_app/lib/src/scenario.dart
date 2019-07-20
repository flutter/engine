// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

/// A scenario to run for testing.
abstract class Scenario {
  /// Creates a new scenario using a specific Window instance.
  const Scenario(this.window);

  /// The window used by this scenario. May be mocked.
  final Window window;

  /// Called by the progrem when a frame is ready to be drawn.
  void onBeginFrame(Duration duration);
}
