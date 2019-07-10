// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:flutter/services.dart';

import 'package:test/test.dart';

void main() {
  final MethodChannel channel = MethodChannel('dev.flutter.engine/test');
  channel.setMethodCallHandler((MethodCall call) async {
    switch(call.method) {
      case 'testMethodChannel': return Color(42).value;
      default: return null;
    }
  });
}
