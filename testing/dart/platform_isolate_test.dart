// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:isolate';
import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('PlatformIsolate', () async {
    expect(PlatformIsolate.isRunningOnPlatformThread(), isFalse);

    final platIsoThread = await PlatformIsolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread(),
        debugName: 'PlatformIsolateA');
    print('plat thread: $platIsoThread');
    expect(platIsoThread, isTrue);

    final platIsoThread2 = await PlatformIsolate.run(
        () async {
          print("Hello from PlatformIsolateB");
          await Future.delayed(Duration(seconds: 1));
          print("Hello from PlatformIsolateB after delay");
          return PlatformIsolate.isRunningOnPlatformThread();
        },
        debugName: 'PlatformIsolateB');
    print('plat thread: $platIsoThread2');
    expect(platIsoThread2, isTrue);

    expect(PlatformIsolate.isRunningOnPlatformThread(), isFalse);

    final isoThread = await Isolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread(),
        debugName: 'IsolateA');
    print('iso thread: $isoThread');
    expect(isoThread, isFalse);

    final isoThread2 = await Isolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread(),
        debugName: 'IsolateB');
    print('iso thread: $isoThread2');
    expect(isoThread2, isFalse);

    expect(PlatformIsolate.isRunningOnPlatformThread(), isFalse);

    /*final platIsoViaIsoThread = await Isolate.run(
        () => PlatformIsolate.run(
            () => PlatformIsolate.isRunningOnPlatformThread(),
            debugName: 'PlatformIsolateC'),
        debugName: 'IsolateC');
    print('plat iso via iso thread: $platIsoViaIsoThread');
    expect(platIsoViaIsoThread, isTrue);*/
  });
}
