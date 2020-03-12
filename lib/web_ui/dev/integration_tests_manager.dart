// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;
import 'package:path/path.dart' as pathlib;

import 'environment.dart';
import 'utils.dart';

class IntegrationTestsManager {
  final String _browser;

  /// Installation directory for browser's driver.
  ///
  /// Always re-install since driver can change frequently.
  /// It usually changes with each the browser version changes.
  /// A better solution would be installing the browser and the driver at the
  /// same time.
  // TODO(nurhan):
  io.Directory _driverDir;

  IntegrationTestsManager(this._browser) {
    _driverDir = io.Directory(
        pathlib.join(environment.webUiRootDir.path, 'drivers', _browser));
  }

  // (DONE) TODO: update the paths on environment.dart.
  // TODO: Go to integration tests directory and get a list of tests to run.

  // (DONE) TODO: Check the browser version. Give warning if not chrome.
  // TODO: Install and run driver for chrome.
  // TODO: Run the tests one by one.
  // TODO: For each run print out success fail. Also print out how to rerun.
  Future<bool> runTests() async {
    print('run testssss: $_browser');
    if (_browser != 'chrome') {
      print('WARNING: integration tests are only suppoted on chrome for now');
      return false;
    } else {
      print('run tests IntegrationTestsManager');
      return prepareDriver();
    }
  }
    Future<bool> _cloneWebInstalllers() async {
      final int exitCode = await runProcess(
        'git',
        <String>[
          'clone',
          'https://github.com/flutter/web_installers.git',
        ],
        workingDirectory: _driverDir.path,
      );

      if (exitCode != 0) {
        io.stderr.writeln(
            'Failed to clone web installers. Exited with exit code $exitCode');
        return false;
      } else {
        return true;
      }
    }

    Future<bool> _runPubGet() async {
      final int exitCode = await runProcess(
        environment.pubExecutable,
        <String>[
          'get',
        ],
        workingDirectory: pathlib.join(
            _driverDir.path, 'web_installers', 'packages', 'web_drivers'),
      );

      if (exitCode != 0) {
        io.stderr
            .writeln('Failed to run pub get. Exited with exit code $exitCode');
        return false;
      } else {
        return true;
      }
    }

    Future<bool> _runDriver() async {
      final int exitCode = await runProcess(
        environment.dartExecutable,
        <String>[
          'lib/web_driver_installer.dart',
          '${_browser}driver',
          '--install-only',
        ],
        workingDirectory: pathlib.join(
            _driverDir.path, 'web_installers', 'packages', 'web_drivers'),
      );

      if (exitCode != 0) {
        io.stderr
            .writeln('Failed to run driver. Exited with exit code $exitCode');
        return false;
      } else {
        return true;
      }
    }

    Future<bool> prepareDriver() async {
      if (_driverDir.existsSync()) {
        _driverDir.deleteSync(recursive: true);
      }

      _driverDir.createSync(recursive: true);

      bool installWebInstallers =  await _cloneWebInstalllers();
      if(installWebInstallers) {
        bool pubGet = await _runPubGet();
        if(pubGet) {
          // test install only for now.
          return await _runDriver();
        }
      }
      return false;
    }

  }

  // List<File> getListOfTests(String browser) {
  //   // Only list the files under web_ui/test.
  //   final Directory testDirectory =
  //       Directory(pathlib.join(environment.webUiRootDir.path, 'test'));

  //   final List<FileSystemEntity> entities =
  //       testDirectory.listSync(recursive: true, followLinks: false);
  //   final List<File> testFilesToBuild = List<File>();

  //   print(
  //       'Listing test files under directory: ${testDirectory.path.toString()}');
  //   int count = 0;
  //   for (FileSystemEntity e in entities) {
  //     if (e is File) {
  //       final String path = e.path.toString();
  //       // Listing only test files and not the test doubles.
  //       if (path.endsWith('_test.dart')) {
  //         // Add Goldens only for Linux Chrome.
  //         if (path.contains('golden_tests') &&
  //             Platform.isLinux &&
  //             browser == 'chrome') {
  //           print('file ${e.path.toString()}');
  //           testFilesToBuild.add(e);
  //           count++;
  //         } else if (!path.contains('golden_tests')) {
  //           print('file ${e.path.toString()}');
  //           testFilesToBuild.add(e);
  //           count++;
  //         } else {
  //           print('file skipped ${e.path.toString()}');
  //         }
  //       }
  //     }
  //   }
  //   print('Listed $count items. ${testFilesToBuild.length}');
  //   return testFilesToBuild;
  // }
