// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:flutter_driver/flutter_driver.dart';
import 'package:integration_test/integration_test_driver_extended.dart' as test;

import 'package:web_test_utils/goldens.dart';
import 'package:web_test_utils/image_compare.dart';

import 'package:image/image.dart';

/// Tolerable pixel difference ratio between the goldens and the screenshots.
///
/// We are allowing a higher difference rate compared to the unit tests (where
/// this rate is set to 0.28), since during the end to end tests there are
/// more components on the screen which are not related to the functionality
/// under test ex: a blinking cursor.
const double kMaxDiffRateFailure = 0.5 / 100; // 0.5%

/// Used for calling `integration_test` package.
///
/// Compared to other similar classes which only included the following call:
/// ```
/// Future<void> main() async => test.integrationDriver();
/// ```
///
/// this method is able to take screenshot.
///
/// It provides an `onScreenshot` callback to the `integrationDriver` method.
/// It also includes options for updating the golden files.
Future<void> runTestWithScreenshots(
    {double diffRateFailure = kMaxDiffRateFailure}) async {
  final WebFlutterDriver driver =
      await FlutterDriver.connect() as WebFlutterDriver;

  // Learn the browser in use from the webDriver.
  final String browser = driver.webDriver.capabilities['browserName'] as String;

  bool updateGoldens = false;
  final String updateGoldensFlag = io.Platform.environment['UPDATE_GOLDENS'];
  if (updateGoldensFlag == null || updateGoldensFlag.toLowerCase() != 'true') {
    // We are using an environment variable since instead of an argument, since
    // this code is not invoked from the shell but from the `flutter drive`
    // tool itself. Therefore we do not have control on the command line
    // arguments.
    // Please read the README, further info on how to update the goldens.
    print('INFO: Goldens will not be updated. Please set `UPDATE_GOLDENS` '
        'environment variable to `true` for updating them.');
  } else {
    updateGoldens = true;
  }

  test.integrationDriver(
    driver: driver,
    onScreenshot: (String screenshotName, List<int> screenshotBytes) async {
      final Image screenshot = decodePng(screenshotBytes);
      final String result = compareImage(
        screenshot,
        updateGoldens,
        '$screenshotName-$browser.png',
        PixelComparison.fuzzy,
        diffRateFailure,
        forIntegrationTests: true,
        write: updateGoldens,
      );
      if (result != 'OK') {
        print('ERROR: $result');
      }
      return result == 'OK';
    },
  );
}
