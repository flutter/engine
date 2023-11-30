// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:isolate';
import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('PlatformIsolate', () async {
    print('this thread: ${PlatformIsolate.getCurrentThreadId()}');
    expect(true, true);
    final platIsoThread = await PlatformIsolate.run(
        () => PlatformIsolate.getCurrentThreadId(),
        debugName: 'PlatformIsolate');
    print('plat thread: $platIsoThread');
    final platIsoThread2 = await PlatformIsolate.run(
        () => PlatformIsolate.getCurrentThreadId(),
        debugName: 'PlatformIsolate');
    print('plat thread: $platIsoThread2');
    print('this thread: ${PlatformIsolate.getCurrentThreadId()}');
    final isoThread = await Isolate.run(
        () => PlatformIsolate.getCurrentThreadId());
    print('iso thread: $isoThread');
    final isoThread2 = await Isolate.run(
        () => PlatformIsolate.getCurrentThreadId());
    print('iso thread: $isoThread2');
    print('this thread: ${PlatformIsolate.getCurrentThreadId()}');
    print('this thread: ${PlatformIsolate.getCurrentThreadId()}');
  });
}
