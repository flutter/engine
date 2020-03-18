// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;
import 'package:path/path.dart' as pathlib;

import 'common.dart';
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
  // (DONE) TODO: Install the driver
  // (DONE) TODO: run driver in the background.
  // (DONE) TODO: Go to the list of integration tests directory and loop for each one.
  // TODO: Fetch flutter for CIs.
  // TODO: Run the tests one by one.
  // TODO: For each run print out success fail. Also print out how to rerun.
  // TODO: Make sure tests finish when they run
  // TODO: Make sure the error is visible.
  Future<bool> run() async {
    if (_browser != 'chrome') {
      print('WARNING: integration tests are only suppoted on chrome for now');
      return false;
    } else {
      bool driverReady = await prepareDriver();
      // TODO(nurhan): provide a flag for running as if on CI. Also use this
      // flag from LUCI.
      if (isCirrus != null && isCirrus) {
        // TODO(nurhan): fetch Flutter, we will use flutter pub get in the next steps.
      }
      if (driverReady) {
        return await _runTests();
      } else {
        return false;
      }
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
      io.stderr.writeln('ERROR: '
          'Failed to clone web installers. Exited with exit code $exitCode');
      return false;
    } else {
      return true;
    }
  }

  Future<bool> _runPubGet(String workingDirectory) async {
    // TODO(nurhan): Run flutter pub get from the fetched flutter on CI.
    final int exitCode = await runProcess(
      //environment.pubExecutable,
      'flutter',
      <String>[
        'pub',
        'get',
      ],
      workingDirectory: workingDirectory,
    );

    if (exitCode != 0) {
      io.stderr.writeln(
          'ERROR: Failed to run pub get. Exited with exit code $exitCode');
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
      io.stderr.writeln(
          'ERROR: Failed to run driver. Exited with exit code $exitCode');
      return false;
    } else {
      runProcessInTheBackground(
        './chromedriver/chromedriver',
        ['--port=4444'],
        workingDirectory: pathlib.join(
            _driverDir.path, 'web_installers', 'packages', 'web_drivers'),
      );
      print('INFO: Driver started');
      return true;
    }
  }

  Future<bool> prepareDriver() async {
    if (_driverDir.existsSync()) {
      _driverDir.deleteSync(recursive: true);
    }

    _driverDir.createSync(recursive: true);

    bool installWebInstallers = await _cloneWebInstalllers();
    if (installWebInstallers) {
      bool pubGet = await _runPubGet(pathlib.join(
          _driverDir.path, 'web_installers', 'packages', 'web_drivers'));
      if (pubGet) {
        return await _runDriver();
      } else {
        return false;
      }
    }
    return false;
  }

  /// Runs the all the web tests under e2e_tests/web.
  Future<bool> _runTests() async {
    // Only list the files under e2e_tests/web.
    final List<io.FileSystemEntity> entities =
        environment.integrationTestsDir.listSync(followLinks: false);

    print('INFO: Listing test files under directory: '
        '${environment.integrationTestsDir.path.toString()}');
    bool testResults = true;
    for (io.FileSystemEntity e in entities) {
      // The tests should be under this directories.
      if (e is io.Directory) {
        testResults = testResults && await _runTestsInTheDirectory(e);
      }
    }
    return testResults;
  }

  /// Run tests in a single directory under: e2e_tests/web.
  ///
  /// Run `flutter pub get` as the first step.
  ///
  /// Validate the directory before running the tests. Each directory is
  /// expected to be a test project which includes a `pubspec.yaml` file
  /// and a `test_driver` directory.
  Future<bool> _runTestsInTheDirectory(io.Directory directory) async {
    // TODO: Validation step. check if the files the directory has.
    // (DONE) (1) pubspec yaml.
    // (DONE) (2) test_driver directory for tests.

    // (DONE) Run pub get.
    // (DONE) Get the list of tests under driver. .dart which has _test.dart.
    // (DONE) Print WARNING: for cases where there is a file missing _test.dart
    // (DONE) Print WARNING: if no tests to run.

    // TODO: Run the tests in debug mode.
    // TODO: print out a statement where to run the test and the command.
    // TODO: Print the result.
    // TODO: return the result as boolean.
    final bool directoryContentValid = _validateTestDirectory(directory);
    if (directoryContentValid) {
      _runPubGet(directory.path);
      _runTestsInDirectory(directory);
    } else {
      return false;
    }
  }

  Future<bool> _runTestsInDirectory(io.Directory directory) async {
    final io.Directory testDirectory =
        io.Directory(pathlib.join(directory.path, 'test_driver'));
    final List<io.FileSystemEntity> entities =
        testDirectory.listSync(followLinks: false);

    final List<String> e2eTestsToRun = List<String>();

    // The following loops over the contents of the directory and saves an
    // expected driver file name for each e2e test assuming any dart file
    // not ending with `_test.dart` is an e2e test.
    // Other files are not considered since developers can add files such as
    // README.
    for (io.File f in entities) {
      final String basename = pathlib.basename(f.path);
      if (!basename.contains('_test.dart') && basename.endsWith('.dart')) {
        e2eTestsToRun.add(basename);
      }
    }

    print(
        'INFO: In project ${directory} ${e2eTestsToRun.length} tests to run.');

    int numberOfPassedTests;
    int numberOfFailedTests;
    for (String fileName in e2eTestsToRun) {
      print('test to run: $fileName');
      final bool testResults = await _runTestsInDebugMode(directory, fileName);
      if (testResults) {
        numberOfPassedTests++;
      } else {
        numberOfFailedTests++;
      }
    }

    final int numberOfTestsRun = numberOfPassedTests + numberOfFailedTests;

    print('${numberOfTestsRun} tests run ${numberOfPassedTests} passed and'
        '${numberOfFailedTests} failed.');

    return numberOfFailedTests == 0;
  }

  Future<bool> _runTestsInDebugMode(
      io.Directory directory, String testName) async {
    // TODO(nurhan): Give options to the developer to run tests in another mode.
    final String statementToRun = 'flutter drive '
        '--target=test_driver/${testName} -d web-server --release '
        '--browser-name=$_browser --local-engine=host_debug_unopt';
    print('INFO: To manually run the test use $statementToRun under '
          'directory ${directory.path}');
    final int exitCode = await runProcess(
      //environment.pubExecutable,
      'flutter',
      <String>[
        'drive',
        '--target=test_driver/${testName}',
        '-d',
        'web-server',
        '--release',
        '--browser-name=$_browser',
        '--local-engine=host_debug_unopt',
      ],
      workingDirectory: directory.path,
    );

    if (exitCode != 0) {
      io.stderr
          .writeln('ERROR: Failed to run test. Exited with exit code $exitCode.'
              ' Statement to run $statementToRun');
      return false;
    } else {
      return true;
    }
  }

  /// Validate the directory has a `pubspec.yaml` file and a `test_driver`
  /// directory.
  ///
  /// Also check the validity of files under `test_driver` directory calling
  /// [_checkE2ETestsValidity] method.
  bool _validateTestDirectory(io.Directory directory) {
    final List<io.FileSystemEntity> entities =
        directory.listSync(followLinks: false);

    // Whether the project has the pubspec.yaml file.
    bool pubSpecFound = false;
    // The test directory 'test_driver'.
    io.Directory testDirectory = null;

    for (io.FileSystemEntity e in entities) {
      // The tests should be under this directories.
      final String baseName = pathlib.basename(e.path);
      if (e is io.Directory && baseName == 'test_driver') {
        testDirectory = e;
      }
      if (e is io.File && baseName == 'pubspec.yaml') {
        pubSpecFound = true;
      }
    }
    if (!pubSpecFound) {
      io.stderr
          .writeln('ERROR: pubspec.yaml file not found in the test project.');
      return false;
    }
    if (testDirectory == null) {
      io.stderr
          .writeln('ERROR: test_driver folder not found in the test project.');
      return false;
    } else {
      return _checkE2ETestsValidity(testDirectory);
    }
  }

  /// Checks if each e2e test file in the directory has a driver test
  /// file to run it.
  ///
  /// Prints informative message to the developer if an error has found.
  /// For each e2e test which has name {name}.dart there will be a driver
  /// file which drives it. The driver file should be named:
  /// {name}_test.dart
  bool _checkE2ETestsValidity(io.Directory testDirectory) {
    final List<io.FileSystemEntity> entities =
        testDirectory.listSync(followLinks: false);

    final Set<String> expectedDriverFileNames = Set<String>();
    final Set<String> foundDriverFileNames = Set<String>();
    int numberOfTests = 0;

    // The following loops over the contents of the directory and saves an
    // expected driver file name for each e2e test assuming any file
    // not ending with `_test.dart` is an e2e test.
    for (io.File f in entities) {
      final String basename = pathlib.basename(f.path);
      print('basename: $basename');
      if (basename.contains('_test.dart')) {
        // First remove this from expectedSet if not there add to the foundSet.
        if (expectedDriverFileNames.contains(basename)) {
          expectedDriverFileNames.remove(basename);
        } else {
          foundDriverFileNames.add(basename);
        }
      } else if (basename.contains('.dart')) {
        // Only run on dart files.
        final String e2efileName =
            basename.substring(0, basename.indexOf('.dart'));
        final String expectedDriverName = '${e2efileName}_test.dart';
        numberOfTests++;
        // First remove this from foundSet if not there add to the expectedSet.
        if (foundDriverFileNames.contains(expectedDriverName)) {
          foundDriverFileNames.remove(expectedDriverName);
        } else {
          expectedDriverFileNames.add(expectedDriverName);
        }
      }
    }

    if (numberOfTests == 0) {
      print('WARNING: No tests to run in this directory.');
    }

    // TODO(nurhan): In order to reduce the work required from team members,
    // remove the need for driver file, by using the same template file.
    // Some driver files are missing.
    if (expectedDriverFileNames.length > 0) {
      for (String expectedDriverName in expectedDriverFileNames) {
        print('ERROR: Test driver file named has ${expectedDriverName} '
            'not found under directory ${testDirectory.path}. Stopping the '
            'integration tests. Please add ${expectedDriverName}. Check to '
            'README file on more details on how to setup integration tests.');
      }
      return false;
    } else {
      return true;
    }
  }
}
