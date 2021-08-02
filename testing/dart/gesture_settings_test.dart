// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('GestureSettings has a reasonable toString', () {
    const GestureSettings gestureSettings = GestureSettings(physicalDoubleTapSlop: 2.0, physicalTouchSlop: 1.0);

    expect(gestureSettings.toString(), '{physicalTouchSlop: 1.0, physicalDoubleTapSlop: 2.0}');
  });

  test('GestureSettings has a correct equality', () {
    // don't refactor these to be const, that defeats the point!
    double value = 2.0;
    final GestureSettings settingsA = GestureSettings(physicalDoubleTapSlop: value, physicalTouchSlop: 1.0);
    final GestureSettings settingsB = GestureSettings(physicalDoubleTapSlop: value, physicalTouchSlop: 3.0);
    final GestureSettings settingsC = GestureSettings(physicalDoubleTapSlop: value, physicalTouchSlop: 1.0);
    value++;

    expect(settingsA, equals(settingsC));
    expect(settingsC, equals(settingsA));

    expect(settingsA, notEquals(settingsB));
    expect(settingsC, notEquals(settingsB));

    expect(settingsB, notEquals(settingsA));
    expect(settingsB, notEquals(settingsC));
  });

  test('GestureSettings copyWith preserves already set values', () {
    const GestureSettings initial = GestureSettings(physicalDoubleTapSlop: 1.0, physicalTouchSlop: 1.0);

    final GestureSettings copyA = initial.copyWith();

    expect(copyA.physicalDoubleTapSlop, 1.0);
    expect(copyA.physicalTouchSlop, 1.0);

    final GestureSettings copyB = copyA.copyWith(physicalDoubleTapSlop: 2.0, physicalTouchSlop: 2.0);

    expect(copyB.physicalDoubleTapSlop, 2.0);
    expect(copyB.physicalTouchSlop, 2.0);
  });

   test('GestureSettings constructor defaults to null', () {
    const GestureSettings settings = GestureSettings();

    expect(settings.physicalDoubleTapSlop, null);
    expect(settings.physicalTouchSlop, null);
  });
}
