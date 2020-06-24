// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:async';
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:meta/meta.dart';
import 'package:path/path.dart' as path;
import 'package:test_core/src/runner/hack_register_platform.dart'
    as hack; // ignore: implementation_imports
import 'package:test_api/src/backend/runtime.dart'; // ignore: implementation_imports
import 'package:test_core/src/executable.dart'
    as test; // ignore: implementation_imports

import 'environment.dart';
import 'exceptions.dart';
import 'integration_tests_manager.dart';
import 'safari_installation.dart';
import 'supported_browsers.dart';
import 'test_platform.dart';
import 'utils.dart';

/// The type of tests requested by the tool user.
enum TestTypesRequested {
  /// For running the unit tests only.
  unit,

  /// For running the integration tests only.
  integration,

  /// For running both unit and integration tests.
  all,
}

class TestCommand extends Command<bool> with ArgUtils {
  TestCommand() {
    argParser
      ..addFlag(
        'debug',
        help: 'Pauses the browser before running a test, giving you an '
            'opportunity to add breakpoints or inspect loaded code before '
            'running the code.',
      )
      ..addFlag(
        'unit-tests-only',
        defaultsTo: false,
        help: 'felt test command runs the unit tests and the integration tests '
            'at the same time. If this flag is set, only run the unit tests.',
      )
      ..addFlag(
        'integration-tests-only',
        defaultsTo: false,
        help: 'felt test command runs the unit tests and the integration tests '
            'at the same time. If this flag is set, only run the integration '
            'tests.',
      )
      ..addFlag('use-system-flutter',
          defaultsTo: false,
          help:
              'integration tests are using flutter repository for various tasks'
              ', such as flutter drive, flutter pub get. If this flag is set, felt '
              'will use flutter command without cloning the repository. This flag '
              'can save internet bandwidth. However use with caution. Note that '
              'since flutter repo is always synced to youngest commit older than '
              'the engine commit for the tests running in CI, the tests results '
              'won\'t be consistent with CIs when this flag is set. flutter '
              'command should be set in the PATH for this flag to be useful.'
              'This flag can also be used to test local Flutter changes.')
      ..addFlag(
        'update-screenshot-goldens',
        defaultsTo: false,
        help:
            'When running screenshot tests writes them to the file system into '
            '.dart_tool/goldens. Use this option to bulk-update all screenshots, '
            'for example, when a new browser version affects pixels.',
      )
      ..addOption(
        'browser',
        defaultsTo: 'chrome',
        help: 'An option to choose a browser to run the tests. Tests only work '
            ' on Chrome for now.',
      );

    SupportedBrowsers.instance.argParsers
        .forEach((t) => t.populateOptions(argParser));
  }

  @override
  final String name = 'test';

  @override
  final String description = 'Run tests.';

  TestTypesRequested testTypesRequested = null;

  /// Check the flags to see what type of tests are requested.
  TestTypesRequested findTestType() {
    if (boolArg('unit-tests-only') && boolArg('integration-tests-only')) {
      throw ArgumentError('Conflicting arguments: unit-tests-only and '
          'integration-tests-only are both set');
    } else if (boolArg('unit-tests-only')) {
      print('Running the unit tests only');
      return TestTypesRequested.unit;
    } else if (boolArg('integration-tests-only')) {
      if (!isChrome && !isSafariOnMacOS && !isFirefox) {
        throw UnimplementedError(
            'Integration tests are only available on Chrome Desktop for now');
      }
      return TestTypesRequested.integration;
    } else {
      return TestTypesRequested.all;
    }
  }

  @override
  Future<bool> run() async {
    SupportedBrowsers.instance
      ..argParsers.forEach((t) => t.parseOptions(argResults));

    // Check the flags to see what type of integration tests are requested.
    testTypesRequested = findTestType();

    switch (testTypesRequested) {
      case TestTypesRequested.unit:
        return runUnitTests();
      case TestTypesRequested.integration:
        return runIntegrationTests();
      case TestTypesRequested.all:
        // TODO(nurhan): https://github.com/flutter/flutter/issues/53322
        // TODO(nurhan): Expand browser matrix for felt integration tests.
        if (runAllTests && (isChrome || isSafariOnMacOS || isFirefox)) {
          bool unitTestResult = await runUnitTests();
          bool integrationTestResult = await runIntegrationTests();
          if (integrationTestResult != unitTestResult) {
            print('Tests run. Integration tests passed: $integrationTestResult '
                'unit tests passed: $unitTestResult');
          }
          return integrationTestResult && unitTestResult;
        } else {
          return await runUnitTests();
        }
    }
    return false;
  }

  Future<bool> runIntegrationTests() async {
    return IntegrationTestsManager(browser, useSystemFlutter).runTests();
  }

  Future<bool> runUnitTests() async {
    _copyTestFontsIntoWebUi();
    await _buildHostPage();
    if (io.Platform.isWindows) {
      // On Dart 2.7 or greater, it gives an error for not
      // recognized "pub" version and asks for "pub" get.
      // See: https://github.com/dart-lang/sdk/issues/39738
      await _runPubGet();
    }

    // In order to run iOS Safari unit tests we need to make sure iOS Simulator
    // is booted.
    if (browser == 'ios-safari') {
      final IosSimulatorManager iosSimulatorManager = IosSimulatorManager();
      IosSimulator iosSimulator;
      try {
        // In order to provide more failure information print simulator info
        iosSimulator = await iosSimulatorManager.getSimulator(
            IosSafariArgParser.instance.iosMajorVersion,
            IosSafariArgParser.instance.iosMinorVersion,
            IosSafariArgParser.instance.iosDevice);
      } catch (e) {
        throw Exception('Error getting requested simulator. Try running '
            '`felt create` command first before running the tests. exception: '
            '$e');
      }

      if (!iosSimulator.booted) {
        await iosSimulator.boot();
        print('INFO: Simulator ${iosSimulator.id} booted.');
        cleanupCallbacks.add(() async {
          await iosSimulator.shutdown();
          print('INFO: Simulator ${iosSimulator.id} shutdown.');
        });
      }
    }

    await _buildTargets();

    if (runAllTests) {
      await _runAllTestsForCurrentPlatform();
    } else {
      await _runSpecificTests(targetFiles);
    }
    return true;
  }

  /// Builds all test targets that will be run.
  Future<void> _buildTargets() async {
    final Stopwatch stopwatch = Stopwatch()..start();
    List<FilePath> allTargets;
    if (runAllTests) {
      allTargets = environment.webUiTestDir
          .listSync(recursive: true)
          .whereType<io.File>()
          .where((io.File f) => f.path.endsWith('_test.dart'))
          .map<FilePath>((io.File f) => FilePath.fromWebUi(
              path.relative(f.path, from: environment.webUiRootDir.path)))
          .toList();
    } else {
      allTargets = targetFiles;
    }

    // Separate HTML targets from CanvasKit targets because the two use
    // different dart2js options (and different build.*.yaml files).
    final List<FilePath> htmlTargets = <FilePath>[];
    final List<FilePath> canvasKitTargets = <FilePath>[];
    final String canvasKitTestDirectory =
        path.join(environment.webUiTestDir.path, 'canvaskit');
    for (FilePath target in allTargets) {
      if (path.isWithin(canvasKitTestDirectory, target.absolute)) {
        canvasKitTargets.add(target);
      } else {
        htmlTargets.add(target);
      }
    }

    if (htmlTargets.isNotEmpty) {
      await _buildTests(targets: htmlTargets, forCanvasKit: false);
    }

    if (canvasKitTargets.isNotEmpty) {
      await _buildTests(targets: canvasKitTargets, forCanvasKit: true);
    }
    stopwatch.stop();
    print('The build took ${stopwatch.elapsedMilliseconds ~/ 1000} seconds.');
  }

  /// Whether to start the browser in debug mode.
  ///
  /// In this mode the browser pauses before running the test to allow
  /// you set breakpoints or inspect the code.
  bool get isDebug => boolArg('debug');

  /// Paths to targets to run, e.g. a single test.
  List<String> get targets => argResults.rest;

  /// The target test files to run.
  ///
  /// The value can be null if the developer prefers to run all the tests.
  List<FilePath> get targetFiles => (targets.isEmpty)
      ? null
      : targets.map((t) => FilePath.fromCwd(t)).toList();

  /// Whether all tests should run.
  bool get runAllTests => targets.isEmpty;

  /// The name of the browser to run tests in.
  String get browser => (argResults != null) ? stringArg('browser') : 'chrome';

  /// Whether [browser] is set to "chrome".
  bool get isChrome => browser == 'chrome';

  /// Whether [browser] is set to "firefox".
  bool get isFirefox => browser == 'firefox';

  /// Whether [browser] is set to "safari".
  bool get isSafariOnMacOS => browser == 'safari'
      && io.Platform.isMacOS;

  /// Use system flutter instead of cloning the repository.
  ///
  /// Read the flag help for more details. Uses PATH to locate flutter.
  bool get useSystemFlutter => boolArg('use-system-flutter');

  /// When running screenshot tests writes them to the file system into
  /// ".dart_tool/goldens".
  bool get doUpdateScreenshotGoldens => boolArg('update-screenshot-goldens');

  /// Runs all tests specified in [targets].
  ///
  /// Unlike [_runAllTestsForCurrentPlatform], this does not filter targets
  /// by platform/browser capabilites, and instead attempts to run all of
  /// them.
  Future<void> _runSpecificTests(List<FilePath> targets) async {
    await _runTestBatch(targets, concurrency: 1, expectFailure: false);
    _checkExitCode();
  }

  /// Runs as many tests as possible on the current OS/browser combination.
  Future<void> _runAllTestsForCurrentPlatform() async {
    final io.Directory testDir = io.Directory(path.join(
      environment.webUiRootDir.path,
      'test',
    ));

    // Screenshot tests and smoke tests only run in Chrome.
    if (isChrome) {
      // Separate screenshot tests from unit-tests. Screenshot tests must run
      // one at a time. Otherwise, they will end up screenshotting each other.
      // This is not an issue for unit-tests.
      final FilePath failureSmokeTestPath = FilePath.fromWebUi(
        'test/golden_tests/golden_failure_smoke_test.dart',
      );
      final List<FilePath> screenshotTestFiles = <FilePath>[];
      final List<FilePath> unitTestFiles = <FilePath>[];

      for (io.File testFile
          in testDir.listSync(recursive: true).whereType<io.File>()) {
        final FilePath testFilePath = FilePath.fromCwd(testFile.path);
        if (!testFilePath.absolute.endsWith('_test.dart')) {
          // Not a test file at all. Skip.
          continue;
        }
        if (testFilePath == failureSmokeTestPath) {
          // A smoke test that fails on purpose. Skip.
          continue;
        }

        if (path.split(testFilePath.relativeToWebUi).contains('golden_tests')) {
          screenshotTestFiles.add(testFilePath);
        } else {
          unitTestFiles.add(testFilePath);
        }
      }

      // This test returns a non-zero exit code on purpose. Run it separately.
      if (io.Platform.environment['CIRRUS_CI'] != 'true') {
        await _runTestBatch(
          <FilePath>[failureSmokeTestPath],
          concurrency: 1,
          expectFailure: true,
        );
        _checkExitCode();
      }

      // Run all unit-tests as a single batch.
      await _runTestBatch(unitTestFiles, concurrency: 10, expectFailure: false);
      _checkExitCode();

      // Run screenshot tests one at a time.
      for (FilePath testFilePath in screenshotTestFiles) {
        await _runTestBatch(
          <FilePath>[testFilePath],
          concurrency: 1,
          expectFailure: false,
        );
        _checkExitCode();
      }
    } else {
      final List<FilePath> unitTestFiles = <FilePath>[];
      for (io.File testFile
          in testDir.listSync(recursive: true).whereType<io.File>()) {
        final FilePath testFilePath = FilePath.fromCwd(testFile.path);
        if (!testFilePath.absolute.endsWith('_test.dart')) {
          // Not a test file at all. Skip.
          continue;
        }
        if (!path
            .split(testFilePath.relativeToWebUi)
            .contains('golden_tests')) {
          unitTestFiles.add(testFilePath);
        }
      }
      // Run all unit-tests as a single batch.
      await _runTestBatch(unitTestFiles, concurrency: 10, expectFailure: false);
      _checkExitCode();
    }
  }

  void _checkExitCode() {
    if (io.exitCode != 0) {
      throw ToolException('Process exited with exit code ${io.exitCode}.');
    }
  }

  Future<void> _runPubGet() async {
    final int exitCode = await runProcess(
      environment.pubExecutable,
      <String>[
        'get',
      ],
      workingDirectory: environment.webUiRootDir.path,
    );

    if (exitCode != 0) {
      throw ToolException(
          'Failed to run pub get. Exited with exit code $exitCode');
    }
  }

  Future<void> _buildHostPage() async {
    final String hostDartPath = path.join('lib', 'static', 'host.dart');
    final io.File hostDartFile = io.File(path.join(
      environment.webEngineTesterRootDir.path,
      hostDartPath,
    ));
    final io.File timestampFile = io.File(path.join(
      environment.webEngineTesterRootDir.path,
      '$hostDartPath.js.timestamp',
    ));

    final String timestamp =
        hostDartFile.statSync().modified.millisecondsSinceEpoch.toString();
    if (timestampFile.existsSync()) {
      final String lastBuildTimestamp = timestampFile.readAsStringSync();
      if (lastBuildTimestamp == timestamp) {
        // The file is still fresh. No need to rebuild.
        return;
      } else {
        // Record new timestamp, but don't return. We need to rebuild.
        print('${hostDartFile.path} timestamp changed. Rebuilding.');
      }
    } else {
      print('Building ${hostDartFile.path}.');
    }

    final int exitCode = await runProcess(
      environment.dart2jsExecutable,
      <String>[
        hostDartPath,
        '-o',
        '$hostDartPath.js',
      ],
      workingDirectory: environment.webEngineTesterRootDir.path,
    );

    if (exitCode != 0) {
      throw ToolException('Failed to compile ${hostDartFile.path}. Compiler '
          'exited with exit code $exitCode');
    }

    // Record the timestamp to avoid rebuilding unless the file changes.
    timestampFile.writeAsStringSync(timestamp);
  }

  /// Builds the specific test [targets].
  ///
  /// [targets] must not be null.
  ///
  /// When building for CanvasKit we have to use a separate `build.canvaskit.yaml`
  /// config file. Otherwise, `build.html.yaml` is used. Because `build_runner`
  /// overwrites the output directories, we redirect the CanvasKit output to a
  /// separate directory, then copy the files back to `build/test`.
  Future<void> _buildTests({List<FilePath> targets, bool forCanvasKit}) async {
    print(
        'Building ${targets.length} targets for ${forCanvasKit ? 'CanvasKit' : 'HTML'}');
    final String canvasKitOutputRelativePath =
        path.join('.dart_tool', 'canvaskit_tests');
    List<String> arguments = <String>[
      'run',
      'build_runner',
      'build',
      '--enable-experiment=non-nullable',
      'test',
      '-o',
      forCanvasKit ? canvasKitOutputRelativePath : 'build',
      '--config',
      // CanvasKit uses `build.canvaskit.yaml`, which HTML Uses `build.html.yaml`.
      forCanvasKit ? 'canvaskit' : 'html',
      for (FilePath path in targets) ...[
        '--build-filter=${path.relativeToWebUi}.js',
        '--build-filter=${path.relativeToWebUi}.browser_test.dart.js',
      ],
    ];

    final int exitCode = await runProcess(
      environment.pubExecutable,
      arguments,
      workingDirectory: environment.webUiRootDir.path,
      environment: <String, String>{
        // This determines the number of concurrent dart2js processes.
        //
        // By default build_runner uses 4 workers.
        //
        // In a testing on a 32-core 132GB workstation increasing this number to
        // 32 sped up the build from ~4min to ~1.5min.
        if (io.Platform.environment.containsKey('BUILD_MAX_WORKERS_PER_TASK'))
          'BUILD_MAX_WORKERS_PER_TASK':
              io.Platform.environment['BUILD_MAX_WORKERS_PER_TASK'],
      },
    );

    if (exitCode != 0) {
      throw ToolException(
          'Failed to compile tests. Compiler exited with exit code $exitCode');
    }

    if (forCanvasKit) {
      final io.Directory canvasKitTemporaryOutputDirectory = io.Directory(
          path.join(environment.webUiRootDir.path, canvasKitOutputRelativePath,
              'test', 'canvaskit'));
      final io.Directory canvasKitOutputDirectory = io.Directory(
          path.join(environment.webUiBuildDir.path, 'test', 'canvaskit'));
      if (await canvasKitOutputDirectory.exists()) {
        await canvasKitOutputDirectory.delete(recursive: true);
      }
      await canvasKitTemporaryOutputDirectory
          .rename(canvasKitOutputDirectory.path);
    }
  }

  /// Runs a batch of tests.
  ///
  /// Unless [expectFailure] is set to false, sets [io.exitCode] to a non-zero value if any tests fail.
  Future<void> _runTestBatch(
    List<FilePath> testFiles, {
    @required int concurrency,
    @required bool expectFailure,
  }) async {
    final List<String> testArgs = <String>[
      ...<String>['-r', 'compact'],
      '--concurrency=$concurrency',
      if (isDebug) '--pause-after-load',
      '--platform=${SupportedBrowsers.instance.supportedBrowserToPlatform[browser]}',
      '--precompiled=${environment.webUiRootDir.path}/build',
      SupportedBrowsers.instance.browserToConfiguration[browser],
      '--',
      ...testFiles.map((f) => f.relativeToWebUi).toList(),
    ];

    hack.registerPlatformPlugin(<Runtime>[
      SupportedBrowsers.instance.supportedBrowsersToRuntimes[browser]
    ], () {
      return BrowserPlatform.start(
        browser,
        root: io.Directory.current.path,
        // It doesn't make sense to update a screenshot for a test that is expected to fail.
        doUpdateScreenshotGoldens: !expectFailure && doUpdateScreenshotGoldens,
      );
    });

    // We want to run tests with `web_ui` as a working directory.
    final dynamic backupCwd = io.Directory.current;
    io.Directory.current = environment.webUiRootDir.path;
    await test.main(testArgs);
    io.Directory.current = backupCwd;

    if (expectFailure) {
      if (io.exitCode != 0) {
        // It failed, as expected.
        io.exitCode = 0;
      } else {
        io.stderr.writeln(
          'Tests ${testFiles.join(', ')} did not fail. Expected failure.',
        );
        io.exitCode = 1;
      }
    }
  }
}

const List<String> _kTestFonts = <String>['ahem.ttf', 'Roboto-Regular.ttf'];

void _copyTestFontsIntoWebUi() {
  final String fontsPath = path.join(
    environment.flutterDirectory.path,
    'third_party',
    'txt',
    'third_party',
    'fonts',
  );

  for (String fontFile in _kTestFonts) {
    final io.File sourceTtf = io.File(path.join(fontsPath, fontFile));
    final String destinationTtfPath =
        path.join(environment.webUiRootDir.path, 'lib', 'assets', fontFile);
    sourceTtf.copySync(destinationTtfPath);
  }
}


// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

/// Manages iOS Simulators.
///
/// Creates a simulator or provides access to an existing simulator by
/// returning [IOSSimulator] instances.
///
/// Uses `xcrun simctl` command to manage the simulators.
///
/// Run `xcrun simctl --help` to learn more on the details of this tool.
class IosSimulatorManager {
  IosSimulatorManager() {
    if (!io.Platform.isMacOS) {
      throw Exception('Platform ${io.Platform.operatingSystem} is not supported'
          '. This class should only be used on macOS. It uses xcrun '
          'simctl command line tool to manage the iOS simulators');
    }
  }

  /// Uses `xcrun simctl create` command to create an iOS Simulator.
  ///
  /// Runs `xcrun simctl list runtimes` to list the runtimes existing on your
  /// macOS. If runtime derived from [majorVersion] and [minorVersion] is not
  /// available an exception will be thrown. Use Xcode to install more versions.
  ///
  /// [device] example iPhone 11 Pro. Run `xcrun simctl list devicetypes` to
  /// list the device types available. If [device] is not available, an
  /// exception will be thrown. Use Xcode to install more versions.
  ///
  /// Use `xcrun simctl create --help` for more details.
  Future<IosSimulator> createSimulator(
      int majorVersion, int minorVersion, String device) async {
    final String runtime = 'iOS ${majorVersion}.${minorVersion}';

    // Check if the runtime is available.
    final io.ProcessResult runtimeListResult =
        await io.Process.run('xcrun', ['simctl', 'list', 'runtimes']);

    if (runtimeListResult.exitCode != 0) {
      throw Exception('Failed to boot list runtimes(versions). Command used: '
          'xcrun simctl list runtimes');
    }

    final String output = runtimeListResult.stdout as String;
    if (!output.contains(runtime)) {
      throw Exception('Mac does not have the requested $runtime '
          'available for simulators. Please use XCode to install.');
    }

    // Check if the device is available.
    final io.ProcessResult deviceListResult =
        await io.Process.run('xcrun', ['simctl', 'list', 'devicetypes']);

    if (deviceListResult.exitCode != 0) {
      throw Exception('Failed to boot list available simulator device types.'
          'Command used: xcrun simctl list devicetypes');
    }

    final String deviceListOutput = deviceListResult.stdout as String;
    if (!deviceListOutput.contains(device)) {
      throw Exception('Mac does not have the requested device type $device '
          'available for simulators. Please use XCode to install.');
    }

    // Prepate device type argument. It should look like:
    // com.apple.CoreSimulator.SimDeviceType.iPhone-11-Pro
    final String deviceTypeAsArg =
        'com.apple.CoreSimulator.SimDeviceType.${device.replaceAll(' ', '-')}';

    // Prepare runtime as argument using the versions. It should look like:
    // com.apple.CoreSimulator.SimRuntime.iOS-13-1.
    final String runtimeTypeAsArg =
        'com.apple.CoreSimulator.SimRuntime.iOS-${majorVersion}-${minorVersion}';

    final io.ProcessResult createResult = await io.Process.run('xcrun',
        ['simctl', 'create', device, deviceTypeAsArg, runtimeTypeAsArg]);

    if (createResult.exitCode != 0) {
      throw Exception('Failed to create requested simulator using $device '
          '$deviceTypeAsArg $runtimeTypeAsArg arguments.');
    }

    // Output will have the simulator id.
    final String simulatorId = createResult.stdout as String;
    return IosSimulator._(false, simulatorId.trim());
  }

  /// Returns an [IosSimulator] instance to control the simulator,
  /// if a simulator corresponding to given [osVersion] and [phone] information
  /// exits.
  ///
  /// Throws if such a simulator is not available.
  Future<IosSimulator> getSimulator(
      int osMajorVersion, int osMinorVersion, String phone) async {
    final String simulatorVersion =
        '-- iOS ${osMajorVersion}.${osMinorVersion} --';
    final String simulatorsList =
        await listExistingSimulators(osMajorVersion, osMinorVersion);

    // The simulator list, have the version string followed by a list of phone
    // names along with their ids and their statuses. Example output:
    // -- iOS 13.5 --
    //   iPhone 8 (2A437C91-3B85-4D7B-BB91-32561DA07B85) (Shutdown)
    //   iPhone 8 Plus (170207A8-7631-4CBE-940E-86A7815AEB2B) (Shutdown)
    //   iPhone 11 (7AEC5FB9-E08A-4F7F-8CA2-1518CE3A3E0D) (Booted)
    //   iPhone 11 Pro (D8074C8B-35A5-4DA5-9AB2-4CE738A5E5FC) (Shutdown)
    //   iPhone 11 Pro Max (3F33AD9A-805E-43E0-A86C-8FC70464A390) (Shutdown)
    // -- iOS 13.6 --
    //   iPhone 8 (2A437C91-3B85-4D7B-BB91-32561DA07B85) (Shutdown)
    //   iPhone 8 Plus (170207A8-7631-4CBE-940E-86A7815AEB2B) (Shutdown)
    //   iPhone 11 (7AEC5FB9-E08A-4F7F-8CA2-1518CE3A3E0D) (Booted)
    //   iPhone 11 Pro (D8074C8B-35A5-4DA5-9AB2-4CE738A5E5FC) (Shutdown)
    final int indexOfVersionListStart =
        simulatorsList.indexOf(simulatorVersion);
    final String restOfTheOutput = simulatorsList
        .substring(indexOfVersionListStart + simulatorVersion.length);
    final int indexOfNextVersion = restOfTheOutput.indexOf('--');
    final String listOfPhones =
        restOfTheOutput.substring(0, indexOfNextVersion);

    final int indexOfPhone = listOfPhones.indexOf(phone);
    if (indexOfPhone == -1) {
      throw Exception('Simulator of $phone is not available for iOS version '
          '${osMajorVersion}.${osMinorVersion}');
    }

    final String phoneInfo = listOfPhones.substring(indexOfPhone);
    final int endIndexOfPhoneId = phoneInfo.indexOf(')');
    final String simulatorId =
        phoneInfo.substring(phoneInfo.indexOf('(') + 1, endIndexOfPhoneId);

    final String phoneInfoAfterId = phoneInfo.substring(endIndexOfPhoneId + 1);
    final String simulatorStatus = phoneInfoAfterId.substring(
        phoneInfoAfterId.indexOf('(') + 1, phoneInfoAfterId.indexOf(')'));
    return IosSimulator._(simulatorStatus == 'Booted', simulatorId);
  }

  Future<String> listExistingSimulators(
      int osMajorVersion, int osMinorVersion) async {
    final io.ProcessResult versionResult =
        await io.Process.run('xcrun', ['simctl', 'list', 'devices']);

    if (versionResult.exitCode != 0) {
      throw Exception('Failed to list iOS simulators.');
    }
    final String output = versionResult.stdout as String;
    // If the requested iOS version simulators exists, there should be a block
    // starting with: `-- iOS osMajorVersion.osMinorVersion --`

    print('output: $output');

    final bool versionCheck =
        output.contains('-- iOS ${osMajorVersion}.${osMinorVersion} --');

    if (!versionCheck) {
      throw Exception(
          'Requested simulator version iOS ${osMajorVersion}.${osMinorVersion} '
          'is not available.');
    }

    return output;
  }
}

/// A class that can be used to boot/shutdown an iOS Simulator.
class IosSimulator {
  final String id;

  bool _booted;
  bool get booted => _booted;

  IosSimulator._(this._booted, this.id);

  /// Boots the iOS Simulator using the simulator [id].
  ///
  /// Uses `xcrun simctl boot` command to boot an iOS Simulator.
  ///
  /// If it is already booted the command will fail.
  Future<void> boot() async {
    final io.ProcessResult versionResult =
        await io.Process.run('xcrun', ['simctl', 'boot', '$id']);

    if (versionResult.exitCode != 0) {
      throw Exception('Failed to boot iOS simulators with id: $id.');
    }
    this._booted = true;
    return;
  }

  /// Shuts down the iOS Simulator using the simulator [id].
  ///
  /// Uses `xcrun simctl shutdown` command to boot an iOS Simulator.
  ///
  /// If the simulator is not booted, the command will fail.
  Future<void> shutdown() async {
    final io.ProcessResult versionResult =
        await io.Process.run('xcrun', ['simctl', 'shutdown', '$id']);

    if (versionResult.exitCode != 0) {
      throw Exception(
          'Failed to shutdown iOS simulators with id: $id.');
    }

    this._booted = false;
    return;
  }

  @override
  String toString() {
    return 'iOS Simulator id: $id status: $booted';
  }
}
