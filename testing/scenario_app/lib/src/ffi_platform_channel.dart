// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'channel_util.dart';
import 'scenario.dart';

/// Tests calling an FfiPlatformChannel
class FfiPlatformChannel extends Scenario {
  /// Constructor.
  FfiPlatformChannel(PlatformDispatcher dispatcher)
      : assert(dispatcher != null),
        super(dispatcher);

  @override
  void onBeginFrame(Duration duration) {
    PlatformDispatcher.instance.sendFfiPlatformMessage('ffi-platform-message', null);
  }
}
