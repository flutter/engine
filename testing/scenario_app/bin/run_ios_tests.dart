// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:engine_repo_tools/engine_repo_tools.dart';
import 'package:meta/meta.dart';
import 'package:path/path.dart' as path;

void main(List<String> args) async {
  if (!io.Platform.isMacOS) {
    io.stderr.writeln('This script is only supported on macOS.');
    io.exitCode = 1;
    return;
  }

  final engine = Engine.tryFindWithin();
  if (engine == null) {
    io.stderr.writeln('Must be run from within the engine repository.');
    io.exitCode = 1;
    return;
  }

  if (args.length > 1 || args.contains('-h') || args.contains('--help')) {
    io.stderr.writeln('Usage: run_ios_tests.dart [ios_engine_variant]');
    io.stderr.writeln(_args.usage);
    io.exitCode = 1;
    return;
  }

  final cleanup = <FutureOr<void> Function()>{};
  final results = _args.parse(args);
  try {
    // Terminate early on SIGINT.
    late final StreamSubscription<void> sigint;
    sigint = io.ProcessSignal.sigint.watch().listen((_) {
      for (final cleanupTask in cleanup) {
        cleanupTask();
      }
      throw Exception('Received SIGINT');
    });
    cleanup.add(sigint.cancel);

    _ensureSimulatorsRotateAutomaticallyForPlatformViewRotationTest();

    final deviceName = results.option('device-name')!;
    _deleteAnyExistingDevices(deviceName: deviceName);
    io.stderr.writeln();

    final deviceIdentifier = results.option('device-identifier')!;
    final osRuntime = results.option('os-runtime')!;
    _createDevice(
      deviceName: deviceName,
      deviceIdentifier: deviceIdentifier,
      osRuntime: osRuntime,
    );
    io.stderr.writeln();

    final String iosEngineVariant;
    if (args.length == 1) {
      iosEngineVariant = args[0];
    } else if (ffi.Abi.current() == ffi.Abi.macosArm64) {
      iosEngineVariant = 'ios_debug_sim_unopt_arm64';
    } else {
      iosEngineVariant = 'ios_debug_sim_unopt';
    }
    io.stderr.writeln('Using engine variant: $iosEngineVariant');
    io.stderr.writeln();

    final (scenarioPath, resultBundle) = _buildResultBundlePath(
      engine: engine,
      iosEngineVariant: iosEngineVariant,
    );
    cleanup.add(() => resultBundle.deleteSync(recursive: true));

    final String dumpXcresultOnFailurePath;
    if (results.option('dump-xcresult-on-failure') case final String path) {
      dumpXcresultOnFailurePath = path;
    } else {
      dumpXcresultOnFailurePath = io.Directory.systemTemp.createTempSync().path;
    }

    final osVersion = results.option('os-version')!;
    if (results.flag('with-skia')) {
      io.stderr.writeln('Running simulator tests with Skia');
      io.stderr.writeln();
      final process = await _runTests(
        outScenariosPath: scenarioPath,
        resultBundlePath: resultBundle.path,
        osVersion: osVersion,
        deviceName: deviceName,
        iosEngineVariant: iosEngineVariant,
      );
      cleanup.add(process.kill);

      if (await process.exitCode != 0) {
        io.stderr.writeln('test failed.');
        io.exitCode = 1;
        final String outputPath = _zipAndStoreFailedTestResults(
          iosEngineVariant: iosEngineVariant,
          resultBundlePath: resultBundle.path,
          storePath: dumpXcresultOnFailurePath,
        );
        io.stderr.writeln('Failed test results are stored at $outputPath');
        return;
      } else {
        io.stderr.writeln('test succcess.');
      }
    }

    if (results.flag('with-impeller')) {
      final process = await _runTests(
        outScenariosPath: scenarioPath,
        resultBundlePath: resultBundle.path,
        osVersion: osVersion,
        deviceName: deviceName,
        iosEngineVariant: iosEngineVariant,
        xcodeBuildExtraArgs: [
          ..._skipTestsForImpeller,
          _infplistFPathForImpeller,
        ],
      );
      cleanup.add(process.kill);

      if (await process.exitCode != 0) {
        io.stderr.writeln('test failed.');
        final String outputPath = _zipAndStoreFailedTestResults(
          iosEngineVariant: iosEngineVariant,
          resultBundlePath: resultBundle.path,
          storePath: dumpXcresultOnFailurePath,
        );
        io.stderr.writeln('Failed test results are stored at $outputPath');
        io.exitCode = 1;
        return;
      } else {
        io.stderr.writeln('test succcess.');
      }
    }
  } on Object catch (anyError) {
    io.stderr.writeln('Unexpected error: $anyError');
    io.exitCode = 1;
  } finally {
    for (final cleanupTask in cleanup) {
      await cleanupTask();
    }
  }
}

final _args = ArgParser()
  ..addFlag(
    'help',
    abbr: 'h',
    help: 'Prints usage information.',
    negatable: false,
  )
  ..addOption(
    'device-name',
    help: 'The name of the iOS simulator device to use.',
    defaultsTo: 'iPhone SE (3rd generation)',
  )
  ..addOption(
    'device-identifier',
    help: 'The identifier of the iOS simulator device to use.',
    defaultsTo:
        'com.apple.CoreSimulator.SimDeviceType.iPhone-SE-3rd-generation',
  )
  ..addOption(
    'os-runtime',
    help: 'The OS runtime of the iOS simulator device to use.',
    defaultsTo: 'com.apple.CoreSimulator.SimRuntime.iOS-17-0',
  )
  ..addOption(
    'os-version',
    help: 'The OS version of the iOS simulator device to use.',
    defaultsTo: '17.0',
  )
  ..addFlag(
    'with-impeller',
    help: 'Whether to use the Impeller backend to run the tests.\n\nCan be '
        'combined with --with-skia to run the test suite with both backends.',
    defaultsTo: true,
  )
  ..addFlag(
    'with-skia',
    help:
        'Whether to use the Skia backend to run the tests.\n\nCan be combined '
        'with --with-impeller to run the test suite with both backends.',
    defaultsTo: true,
  )
  ..addOption(
    'dump-xcresult-on-failure',
    help: 'The path to dump the xcresult bundle to if the test fails.\n\n'
          'Defaults to the environment variable FLUTTER_TEST_OUTPUTS_DIR, '
          'otherwise to a randomly generated temporary directory.',
    defaultsTo: io.Platform.environment['FLUTTER_TEST_OUTPUTS_DIR'],
  );

void _ensureSimulatorsRotateAutomaticallyForPlatformViewRotationTest() {
  // Can also be set via Simulator Device > Rotate Device Automatically.
  final result = io.Process.runSync(
    'defaults',
    const [
      'write',
      'com.apple.iphonesimulator',
      'RotateWindowWhenSignaledByGuest',
      '-int 1',
    ],
  );
  if (result.exitCode != 0) {
    throw Exception(
      'Failed to enable automatic rotation for iOS simulator: ${result.stderr}',
    );
  }
}

void _deleteAnyExistingDevices({required String deviceName}) {
  io.stderr
      .writeln('Deleting any existing simulator devices named $deviceName...');

  bool deleteSimulator() {
    final result = io.Process.runSync(
      'xcrun',
      ['simctl', 'delete', deviceName],
    );
    if (result.exitCode == 0) {
      io.stderr.writeln('Deleted $deviceName');
      return true;
    } else {
      return false;
    }
  }

  while (deleteSimulator()) {}
}

void _createDevice({
  required String deviceName,
  required String deviceIdentifier,
  required String osRuntime,
}) {
  io.stderr.writeln('Creating $deviceName $deviceIdentifier $osRuntime...');
  final result = io.Process.runSync(
    'xcrun',
    [
      'simctl',
      'create',
      deviceName,
      deviceIdentifier,
      osRuntime,
    ],
  );
  if (result.exitCode != 0) {
    throw Exception('Failed to create simulator device: ${result.stderr}');
  }
}

@useResult
(String scenarios, io.Directory resultBundle) _buildResultBundlePath({
  required Engine engine,
  required String iosEngineVariant,
}) {
  final scenarioPath = path.normalize(path.join(
    engine.outDir.path,
    iosEngineVariant,
    'scenario_app',
    'Scenarios',
  ));

  // Create a temporary directory to store the test results.
  final result = io.Directory(scenarioPath).createTempSync('ios_scenario_xcresult');
  return (scenarioPath, result);
}

@useResult
Future<io.Process> _runTests({
  required String resultBundlePath,
  required String outScenariosPath,
  required String osVersion,
  required String deviceName,
  required String iosEngineVariant,
  List<String> xcodeBuildExtraArgs = const [],
}) async {
  return io.Process.start(
    'xcodebuild',
    [
      '-project',
      path.join(outScenariosPath, 'Scenarios.xcodeproj'),
      '-sdk',
      'iphonesimulator',
      '-scheme',
      'Scenarios',
      '-resultBundlePath',
      path.join(resultBundlePath, 'ios_scenario.xcresult'),
      '-destination',
      'platform=iOS Simulator,OS=$osVersion,name=$deviceName',
      'clean',
      'test',
      'FLUTTER_ENGINE=$iosEngineVariant',
      ...xcodeBuildExtraArgs,
    ],
    mode: io.ProcessStartMode.inheritStdio,
  );
}

/// -skip-testing {$name} args required to pass the Impeller tests.
///
/// - Skip testFontRenderingWhenSuppliedWithBogusFont: https://github.com/flutter/flutter/issues/113250
/// - Skip golden tests that use software rendering: https://github.com/flutter/flutter/issues/131888
final _skipTestsForImpeller = [
  'ScenariosUITests/MultiplePlatformViewsBackgroundForegroundTest/testPlatformView',
  'ScenariosUITests/MultiplePlatformViewsTest/testPlatformView',
  'ScenariosUITests/NonFullScreenFlutterViewPlatformViewUITests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipPathTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipPathWithTransformTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipRectAfterMovedTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipRectTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipRectWithTransformTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipRRectTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationClipRRectWithTransformTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationLargeClipRRectTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationLargeClipRRectWithTransformTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationOpacityTests/testPlatformView',
  'ScenariosUITests/PlatformViewMutationTransformTests/testPlatformView',
  'ScenariosUITests/PlatformViewRotation/testPlatformView',
  'ScenariosUITests/PlatformViewUITests/testPlatformView',
  'ScenariosUITests/PlatformViewWithNegativeOtherBackDropFilterTests/testPlatformView',
  'ScenariosUITests/PlatformViewWithOtherBackdropFilterTests/testPlatformView',
  'ScenariosUITests/RenderingSelectionTest/testSoftwareRendering',
  'ScenariosUITests/TwoPlatformViewClipPathTests/testPlatformView',
  'ScenariosUITests/TwoPlatformViewClipRectTests/testPlatformView',
  'ScenariosUITests/TwoPlatformViewClipRRectTests/testPlatformView',
  'ScenariosUITests/TwoPlatformViewsWithOtherBackDropFilterTests/testPlatformView',
  'ScenariosUITests/UnobstructedPlatformViewTests/testMultiplePlatformViewsWithOverlays',
].map((name) => '-skip-testing $name').toList();

/// Plist with `FTEEnableImpeller=YES`; all projects in the workspace require this file.
///
/// For example, `FlutterAppExtensionTestHost` has a dummy file under the below directory.
final _infplistFPathForImpeller = path.join('Scenarios', 'Info_Impeller.plist');

@useResult
String _zipAndStoreFailedTestResults({
  required String iosEngineVariant,
  required String resultBundlePath,
  required String storePath,
}) {
  final outputPath = path.join(storePath, '$iosEngineVariant.zip');
  final result = io.Process.runSync(
    'zip',
    [
      '-q',
      '-r',
      outputPath,
      resultBundlePath,
    ],
  );
  if (result.exitCode != 0) {
    throw Exception('Failed to zip the test results: ${result.stderr}');
  }
  return outputPath;
}
