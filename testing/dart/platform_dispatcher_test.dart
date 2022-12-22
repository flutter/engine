// Copyright 2022 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('ViewConfiguration asserts that both window and view are not provided', () async {
    expectAssertion(() {
      return ViewConfiguration(
      // ignore: deprecated_member_use
        window: PlatformDispatcher.instance.views.first,
        view: PlatformDispatcher.instance.views.first,
      );
    });
  });

  test("A ViewConfiguration's view and window are backed with the same property", () async {
    final FlutterView view = PlatformDispatcher.instance.views.first;
    final ViewConfiguration viewConfiguration = ViewConfiguration(view: view);
    // ignore: deprecated_member_use
    expect(viewConfiguration.window, view);
    // ignore: deprecated_member_use
    expect(viewConfiguration.window, viewConfiguration.view);
  });

  test('Calling copyWith on a ViewConfiguration with both a window and view throws an error', () async {
    final FlutterView view = PlatformDispatcher.instance.views.first;
    expectAssertion(() {
      // ignore: deprecated_member_use
      return ViewConfiguration(view: view)..copyWith(view: view, window: view);
    });
  });
}
