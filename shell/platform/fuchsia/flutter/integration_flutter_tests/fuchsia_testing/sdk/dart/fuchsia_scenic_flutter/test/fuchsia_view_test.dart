// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mockito/mockito.dart';

import 'package:fuchsia_scenic_flutter/fuchsia_view.dart';

void main() {
  testWidgets('FuchsiaView', (tester) async {
    final controller = MockFuchsiaViewController();
    final completer = Completer();
    when(controller.viewId).thenReturn(42);
    when(controller.whenConnected).thenAnswer((_) => Future<bool>.value(false));
    when(controller.connect()).thenAnswer((_) => completer.future);

    await tester.pumpWidget(
      Center(
        child: SizedBox(
          child: FuchsiaView(controller: controller),
        ),
      ),
    );

    completer.complete();
    await tester.pumpAndSettle();

    verify(controller.connect(hitTestable: true, focusable: true));

    // Change properties on the view.
    when(controller.connected).thenReturn(true);

    await tester.pumpWidget(
      Center(
        child: SizedBox(
          child: FuchsiaView(controller: controller, hitTestable: false),
        ),
      ),
    );
    await tester.pumpAndSettle();

    verify(controller.update(hitTestable: false));
  });

  testWidgets('FuchsiaView with args', (tester) async {
    final controller = MockFuchsiaViewController();
    final completer = Completer();
    when(controller.viewId).thenReturn(42);
    when(controller.whenConnected).thenAnswer((_) => Future<bool>.value(false));
    when(controller.connect(hitTestable: false, focusable: false))
        .thenAnswer((_) => completer.future);

    await tester.pumpWidget(
      Center(
        child: SizedBox(
          child: FuchsiaView(
            controller: controller,
            hitTestable: false,
            focusable: false,
          ),
        ),
      ),
    );

    completer.complete();
    await tester.pumpAndSettle();

    verify(controller.connect(hitTestable: false, focusable: false));
  });
}

class MockFuchsiaViewController extends Mock implements FuchsiaViewController {}
