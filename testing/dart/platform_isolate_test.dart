// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:isolate';
import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('PlatformIsolate', () async {
    print('this thread: ${PlatformIsolate.isRunningOnPlatformThread()}');
    expect(PlatformIsolate.isRunningOnPlatformThread(), isFalse);
    final platIsoThread = await PlatformIsolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread(),
        debugName: 'PlatformIsolate');
    print('plat thread: $platIsoThread');
    expect(platIsoThread, isTrue);
    final platIsoThread2 = await PlatformIsolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread(),
        debugName: 'PlatformIsolate');
    print('plat thread: $platIsoThread2');
    expect(platIsoThread2, isTrue);
    print('this thread: ${PlatformIsolate.isRunningOnPlatformThread()}');
    final isoThread = await Isolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread());
    print('iso thread: $isoThread');
    expect(isoThread, isFalse);
    final isoThread2 = await Isolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread());
    print('iso thread: $isoThread2');
    expect(isoThread2, isFalse);
    expect(PlatformIsolate.isRunningOnPlatformThread(), isFalse);
  });
}
