// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_relative_lib_imports

import 'dart:ui';

import 'package:litetest/litetest.dart';
import '../../lib/gpu/gpu.dart' as gpu;

void main() {
  test('no crash', () async {
    final int result = gpu.testProc();
    expect(result, 1);

    final gpu.FlutterGpuTestClass a = gpu.FlutterGpuTestClass();
    a.coolMethod(9847);
  });
}
