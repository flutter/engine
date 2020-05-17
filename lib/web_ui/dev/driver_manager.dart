// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:path/path.dart' as pathlib;
import 'package:web_driver_installer/chrome_driver_installer.dart';

import 'chrome_installer.dart';
import 'common.dart';
import 'environment.dart';
import 'utils.dart';

class ChromeDriverManager extends DriverManager {
  ChromeDriverManager(String browser) : super(browser);

  Future<void> _installDriver() async {
    if (_browserDriverDir.existsSync()) {
      _browserDriverDir.deleteSync(recursive: true);
    }

    _browserDriverDir.createSync(recursive: true);
    temporaryDirectories.add(_drivers);

    io.Directory temp = io.Directory.current;
    io.Directory.current = _browserDriverDir;

    // TODO(nurhan): https://github.com/flutter/flutter/issues/53179
    final String chromeDriverVersion = await queryChromeDriverVersion();
    ChromeDriverInstaller chromeDriverInstaller =
        ChromeDriverInstaller.withVersion(chromeDriverVersion);
    await chromeDriverInstaller.install(alwaysInstall: true);
    io.Directory.current = temp;
  }

  /// Driver should already exist on LUCI as a CIPD package.
  ///
  /// Throw an error if directory does not exists.
  void _verifyDriverForLUCI() {
    if (!_browserDriverDir.existsSync()) {
      throw StateError('Failed to locate Chrome driver on LUCI on path:'
          '${_browserDriverDir.path}');
    }
  }

  Future<void> _startDriver(String driverPath) async {
    await startProcess('./chromedriver/chromedriver', ['--port=4444'],
        workingDirectory: driverPath);
    print('INFO: Driver started');
  }
}

// class SafariDriverManager extends DriverManager {
//   SafariDriverManager(String browser) : super(browser);

//   Future<void> _installDriver() {}

//   Future<void> _verifyDriverForLUCI() {}

//   Future<void> _startDriver(String driverPath) {}
// }

abstract class DriverManager {
  /// Installation directory for browser's driver.
  ///
  /// Always re-install since driver can change frequently.
  /// It usually changes with each the browser version changes.
  /// A better solution would be installing the browser and the driver at the
  /// same time.
  // TODO(nurhan): https://github.com/flutter/flutter/issues/53179. Partly
  // solved. Remaining local integration tests using the locked Chrome version.
  final io.Directory _browserDriverDir;

  /// This is the parent directory for all drivers.
  ///
  /// This directory is saved to [temporaryDirectories] and deleted before
  /// tests shutdown.
  final io.Directory _drivers;

  final String _browser;

  DriverManager(this._browser)
      : this._browserDriverDir = io.Directory(pathlib.join(
            environment.webUiDartToolDir.path,
            'drivers',
            _browser,
            '${_browser}driver-${io.Platform.operatingSystem.toString()}')),
        this._drivers = io.Directory(
            pathlib.join(environment.webUiDartToolDir.path, 'drivers'));

  Future<void> prepareDriver() async {
    if (!isLuci) {
      // LUCI installs driver from CIPD, so we skip installing it on LUCI.
      await _installDriver();
    } else {
      await _verifyDriverForLUCI();
    }
    await _startDriver(_browserDriverDir.path);
  }

  Future<void> _installDriver();

  void _verifyDriverForLUCI();

  Future<void> _startDriver(String driverPath);
}
