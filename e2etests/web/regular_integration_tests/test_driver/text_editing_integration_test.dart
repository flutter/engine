// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_driver/flutter_driver.dart';
import 'package:integration_test/integration_test_driver_extended.dart' as test;

Future<void> main() async {
  final FlutterDriver driver = await FlutterDriver.connect();
  test.integrationDriver(
    driver: driver,
    onScreenshot: (String screenshotName, List<int> screenshotBytes) async {
      return true;
    },
  );
}
