// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'channel_util.dart';
import 'scenario.dart';

/// A blank page that just sends back to the platform what the set initial
/// route is.
class InitialRouteReply extends Scenario {
  /// Creates the InitialRouteReply.
  ///
  /// The [window] parameter must not be null.
  InitialRouteReply(PlatformDispatcher dispatcher)
      : assert(dispatcher != null),
        super(dispatcher);

  @override
  Future<void> onBeginFrame(Duration duration) async {
    sendJsonMethodCall(
      dispatcher: dispatcher,
      channel: 'initial_route_test_channel',
      method: window.defaultRouteName,
    );
  }
}
