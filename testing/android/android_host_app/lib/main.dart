// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:flutter/services.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MaterialApp(home: Container(color: Colors.blue)));
  final MethodChannel channel = MethodChannel('dev.flutter.engine/test');
  channel.invokeMethod('testMethodChannel', Color(42).value);
}
