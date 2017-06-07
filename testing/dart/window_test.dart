// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';
import 'dart:ui';

import 'package:test/test.dart';

void main() {
  group('platform messages', () {
    test('are queued by default', () {
      window.onPlatformMessage = null;
      window.onPlatformMessage('msg1', null, null);
      window.onPlatformMessage('msg2', null, null);
      final List<String> messages = <String>[];
      window.onPlatformMessage = (String name, ByteData _, PlatformMessageResponseCallback __) {
        messages.add(name);
      };
      expect(messages, orderedEquals(<String>['msg1', 'msg2']));
    });
  });
}
