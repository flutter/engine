// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'package:flutter_test/flutter_test.dart';
import 'package:regular_integration_tests/treeshaking_main.dart' as app;
import 'package:flutter/material.dart';

import 'package:e2e/e2e.dart';

void main() {
  E2EWidgetsFlutterBinding.ensureInitialized() as E2EWidgetsFlutterBinding;

  testWidgets('debug+Fill+Properties is tree shaken',
          (WidgetTester tester) async {
    await testIsTreeShaken(tester, '${debugPrefix}FillProperties');
  });
}

// Used to prevent compiler optimization that will generate const string.
String get debugPrefix => <String>['d','e','b','u','g'].join('');

Future<void> testIsTreeShaken(WidgetTester tester, String methodName) async {
  app.main();
  await tester.pumpAndSettle();

  // Make sure app loaded.
  final Finder finder = find.byKey(const Key('mainapp'));
  expect(finder, findsOneWidget);

  await _loadBundleAndCheck(methodName);
}

String fileContents;

Future<void> _loadBundleAndCheck(String methodName) async {
  fileContents ??= await html.HttpRequest.getString('main.dart.js');
  expect(fileContents, contains('RenderObjectToWidgetElement'));
  expect(fileContents.contains(methodName), false);
}
