// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
import 'dart:typed_data';

import 'package:ui/src/engine.dart' hide window;
import 'package:ui/ui.dart';

/// Encapsulates the info of a platform message that was intercepted by
/// [PlatformMessagesSpy].
class PlatformMessage {
  PlatformMessage(this.channel, this.methodCall);

  /// The name of the channel on which the message was sent.
  final String channel;

  /// The [MethodCall] instance that was sent in the platform message.
  final MethodCall methodCall;

  /// Shorthand for getting the name of the method call.
  String get methodName => methodCall.method;

  /// Shorthand for getting the arguments of the method call.
  dynamic get methodArguments => methodCall.arguments;
}

/// Intercepts platform messages sent from the engine to the framework.
///
/// It holds all intercepted platform messages in a [messages] list that can
/// be inspected in tests.
class PlatformMessagesSpy {
  PlatformMessageCallback? _callback;
  PlatformMessageCallback? _backup;

  bool get _isActive => _callback != null;

  /// List of intercepted messages since the last [setUp] call.
  final List<PlatformMessage> messages = <PlatformMessage>[];

  /// Start spying on platform messages.
  ///
  /// This is typically called inside a test's `setUp` callback.
  void setUp() {
    assert(!_isActive);
    _callback = (String channel, ByteData? data,
        PlatformMessageResponseCallback? callback) {
      messages.add(PlatformMessage(
        channel,
        const JSONMethodCodec().decodeMethodCall(data),
      ));
    };

    _backup = window.onPlatformMessage;
    window.onPlatformMessage = _callback;
  }

  /// Stop spying on platform messages and clear all intercepted messages.
  ///
  /// Make sure this is called after each test that uses [PlatformMessagesSpy].
  void tearDown() {
    assert(_isActive);
    // Make sure [window.onPlatformMessage] wasn't tampered with.
    assert(window.onPlatformMessage == _callback);
    _callback = null;
    messages.clear();
    window.onPlatformMessage = _backup;
  }
}
