// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:io' as io;
import 'dart:convert';
import 'dart:math';

import 'package:image/image.dart';
import 'package:webkit_inspection_protocol/webkit_inspection_protocol.dart'
    as wip;

import 'common.dart';

/// [DriverManager] implementation for Chrome.
///
/// This manager can be used for both macOS and Linux.
class ChromeScreenshotManager extends ScreenshotManager {
  String get filenameSuffix => '';

  Future<Image> capture(Map<String, dynamic> region) async {
    final wip.ChromeConnection chromeConnection =
        wip.ChromeConnection('localhost', kDevtoolsPort);
    final wip.ChromeTab chromeTab = await chromeConnection.getTab(
        (wip.ChromeTab chromeTab) => chromeTab.url.contains('localhost'));
    final wip.WipConnection wipConnection = await chromeTab.connect();

    Map<String, dynamic> captureScreenshotParameters = null;
    if (region != null) {
      captureScreenshotParameters = <String, dynamic>{
        'format': 'png',
        'clip': <String, dynamic>{
          'x': region['x'],
          'y': region['y'],
          'width': region['width'],
          'height': region['height'],
          'scale':
              1, // This is NOT the DPI of the page, instead it's the "zoom level".
        },
      };
    }

    // Setting hardware-independent screen parameters:
    // https://chromedevtools.github.io/devtools-protocol/tot/Emulation
    await wipConnection
        .sendCommand('Emulation.setDeviceMetricsOverride', <String, dynamic>{
      'width': kMaxScreenshotWidth,
      'height': kMaxScreenshotHeight,
      'deviceScaleFactor': 1,
      'mobile': false,
    });
    final wip.WipResponse response = await wipConnection.sendCommand(
        'Page.captureScreenshot', captureScreenshotParameters);

    // Compare screenshots
    final Image screenshot =
        decodePng(base64.decode(response.result['data'] as String));

    return screenshot;
  }
}

/// [DriverManager] implementation for Safari.
///
/// This manager is will only be created/used for macOS.
class IOSSafariScreenshotManager extends ScreenshotManager {
  final Random random = Random();

  String get filenameSuffix => '_iOS_Safari';

  /// Part to crop from the top of the image.
  ///
  /// Simulator takes the screenshot of the entire simulator. We are cropping
  /// top bit from the simulator. Otherwise due to the clock on top of the
  /// screen, the screenshot will differ between each run.
  /// TODO: Find a better method since these needs to be updated whenever we
  /// change the phone version of the simulator.
  static final int topGap = 280;

  Future<Image> capture(Map<String, dynamic> region) async {
    final String suffix = random.nextInt(100).toString();
    final io.ProcessResult versionResult = await io.Process.run('xcrun',
        ['simctl', 'io', 'booted', 'screenshot', 'screenshot${suffix}.png']);

    if (versionResult.exitCode != 0) {
      throw Exception('Failed to run xcrun screenshot on iOS simulator.');
    }
    final io.File file = io.File('screenshot${suffix}.png');
    List<int> imageBytes;
    if (file.existsSync()) {
      imageBytes = await file.readAsBytes();
    } else {
      throw Exception('Failed to read the screenshot screenshot${suffix}.png.');
    }

    final Image screenshot = decodePng(imageBytes);
    file.deleteSync();

    return copyCrop(screenshot, region['x'], topGap + region['y'],
        region['width'], region['height']);
  }
}

/// Abstract class for taking screenshots in different browsers.
abstract class ScreenshotManager {
  /// Capture a screenshot of the screen.
  Future<Image> capture(Map<String, dynamic> region);

  /// Suffix to be added to the end of the filename.
  ///
  /// Example file names:
  /// - Chrome, no-suffix: backdrop_filter_clip_moved.actual.png
  /// - iOS Safari: backdrop_filter_clip_moved.actual_iOS_Safari.png
  String get filenameSuffix;

  static ScreenshotManager choose(String browser) {
    if (browser == 'chrome') {
      return ChromeScreenshotManager();
    } else if (browser == 'ios-safari') {
      return IOSSafariScreenshotManager();
    } else {
      throw StateError('Screenshot tests are only supported on Chrome and on '
          'iOS Safari');
    }
  }
}
