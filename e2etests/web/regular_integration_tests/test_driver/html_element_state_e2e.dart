// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import 'package:e2e/e2e.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:regular_integration_tests/html_element_state.dart' as app;

void main() {
  E2EWidgetsFlutterBinding.ensureInitialized() as E2EWidgetsFlutterBinding;

  testWidgets('HtmlElementView does not lose state.', (WidgetTester tester) async {
    app.main();
    await tester.pumpAndSettle();
    final html.DivElement wrapper = html.querySelector('flt-platform-view').shadowRoot.getElementById('wrapper') as html.DivElement;
    wrapper.scrollTop = 400;
    expect(wrapper.scrollTop, 400);
    await tester.tap(find.byKey(const Key('openDrawer')));
    await tester.pumpAndSettle();
    expect(wrapper.scrollTop, 400);
    await tester.tap(find.byKey(const Key('closeDrawer')));
    await tester.pumpAndSettle();
    expect(wrapper.scrollTop, 400);
  });
}
