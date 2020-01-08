// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:ui/src/engine.dart' hide window;
import 'package:ui/ui.dart';

class PlatformMessage {
  PlatformMessage(this.channel, this.methodCall);

  final String channel;
  final MethodCall methodCall;

  String get methodName => methodCall.method;
  String get methodArguments => methodCall.arguments;
}

class PlatformMessagesSpy {
  bool _isActive = false;
  PlatformMessageCallback _backup;

  final List<PlatformMessage> messages = <PlatformMessage>[];

  void activate() {
    assert(!_isActive);
    _isActive = true;
    _backup = window.onPlatformMessage;
    window.onPlatformMessage = (String channel, ByteData data,
        PlatformMessageResponseCallback callback) {
      messages.add(PlatformMessage(
        channel,
        const JSONMethodCodec().decodeMethodCall(data),
      ));
    };
  }

  void deactivate() {
    assert(_isActive);
    _isActive = false;
    messages.clear();
    window.onPlatformMessage = _backup;
  }
}
