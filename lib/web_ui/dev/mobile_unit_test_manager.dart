// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'driver_runner.dart';
import 'environment.dart';
import 'utils.dart';

/// Unit test manager for IOS Safari and Chrome on Android.
class MobileUnitTestManager {
  final String _browser;

  MobileUnitTestManager(this._browser);

  Future<void> runUnitTests() async {
    // TODO(nurhan): Add Chrome on Android.
    // https://github.com/flutter/flutter/issues/53693
    assert(_browser == 'ios-safari');
    // TODO(nurhan): add an option for building a single target or a set
    // of targets. The option is not available in `webdev` yet.
    await _buildTestsWebdev();
    await _serveTests();

    // TODO(nurhan): Start Safari Driver. Implement after functionality is
    // added to web_installers.
    await UnitTestRunner().runWebDriverBasedTests(_browser);
  }

  Future<void> _buildTestsWebdev() async {
    await runProcess('webdev', ['build', '-o', 'build'],
        workingDirectory: environment.webUiRootDir.path);
  }

  Future<void> _serveTests() async {
    await startProcess('webdev', ['serve', 'test:8080'],
        workingDirectory: environment.webUiRootDir.path);
  }
}
