// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@TestOn('vm && linux')

import 'package:test/test.dart';

import 'integration_tests_manager.dart';

void main() async {
  IntegrationTestsManager testsManager;
  setUpAll(() {
    testsManager = IntegrationTestsManager('chrome');
  });

  tearDown(() {
  });

  test('run an install chrome driver', () async {
    testsManager.prepareDriver();
  });
}
