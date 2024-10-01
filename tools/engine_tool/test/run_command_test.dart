// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi' show Abi;
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:engine_repo_tools/engine_repo_tools.dart';
import 'package:engine_tool/src/commands/run_command.dart';
import 'package:engine_tool/src/environment.dart';
import 'package:engine_tool/src/flutter_tool_interop/device.dart';
import 'package:engine_tool/src/flutter_tool_interop/flutter_tool.dart';
import 'package:engine_tool/src/logger.dart';
import 'package:platform/platform.dart';
import 'package:process_fakes/process_fakes.dart';
import 'package:process_runner/process_runner.dart';
import 'package:test/test.dart';

import 'src/test_build_configs.dart';

void main() {
  late io.Directory tempRoot;
  late TestEngine testEngine;

  setUp(() {
    tempRoot = io.Directory.systemTemp.createTempSync('engine_tool_test');
    testEngine = TestEngine.createTemp(rootDir: tempRoot);
  });

  tearDown(() {
    tempRoot.deleteSync(recursive: true);
  });

  test('fails if flutter is not on your PATH', () async {
    final failsCanRun = FakeProcessManager(
      canRun: (executable, {workingDirectory}) {
        if (executable == 'flutter') {
          return false;
        }
        fail('Unexpected');
      },
    );

    final testEnvironment = Environment(
      abi: Abi.macosArm64,
      engine: testEngine,
      logger: Logger.test((_) {}),
      platform: _fakePlatform(Platform.linux),
      processRunner: ProcessRunner(
        defaultWorkingDirectory: tempRoot,
        processManager: failsCanRun,
      ),
    );

    final et = _engineTool(
      RunCommand(
        environment: testEnvironment,
        configs: {},
      ),
    );

    expect(
      () => et.run(['run']),
      throwsA(
        isA<FatalError>().having(
          (a) => a.toString(),
          'toString()',
          contains('"flutter" command in your PATH'),
        ),
      ),
    );
  });

  group('configuration failures', () {
    final unusedProcessManager = FakeProcessManager(
      canRun: (_, {workingDirectory}) => true,
    );

    late List<LogRecord> testLogs;
    late Environment testEnvironment;
    late _FakeFlutterTool flutterTool;

    setUp(() {
      testLogs = [];
      testEnvironment = Environment(
        abi: Abi.linuxX64,
        engine: testEngine,
        logger: Logger.test(testLogs.add),
        platform: _fakePlatform(Platform.linux),
        processRunner: ProcessRunner(
          defaultWorkingDirectory: tempRoot,
          processManager: unusedProcessManager,
        ),
      );
      flutterTool = _FakeFlutterTool();
    });

    test('fails if a target build could not be found', () async {
      final builders = TestBuilderConfig();
      builders.addBuild(
        name: 'linux/android_debug_arm64',
        dimension: TestDroneDimension.linux,
      );

      final et = _engineTool(RunCommand(
        environment: testEnvironment,
        configs: {
          'linux_test_config': builders.buildConfig(
            path: 'ci/builders/linux_test_config.json',
          ),
        },
        flutterTool: flutterTool,
      ));

      expect(
        () => et.run(['run', '--config=android_debug_arm64']),
        throwsA(
          isA<FatalError>().having(
            (a) => a.toString(),
            'toString()',
            contains('Could not find build'),
          ),
        ),
      );
    });

    test('fails if a host build could not be found', () async {
      final builders = TestBuilderConfig();
      builders.addBuild(
        name: 'linux/android_debug_arm64',
        dimension: TestDroneDimension.linux,
        targetDir: 'android_debug_arm64',
      );

      final et = _engineTool(RunCommand(
        environment: testEnvironment,
        configs: {
          'linux_test_config': builders.buildConfig(
            path: 'ci/builders/linux_test_config.json',
          ),
        },
        flutterTool: flutterTool,
      ));

      expect(
        () => et.run(['run', '--config=android_debug_arm64']),
        throwsA(
          isA<FatalError>().having(
            (a) => a.toString(),
            'toString()',
            contains('Could not find host build'),
          ),
        ),
      );
    });

    test('fails if RBE was requested but no RBE config was found', () async {
      final builders = TestBuilderConfig();
      builders.addBuild(
        name: 'linux/android_debug_arm64',
        dimension: TestDroneDimension.linux,
        targetDir: 'android_debug_arm64',
      );
      builders.addBuild(
        name: 'linux/host_debug',
        dimension: TestDroneDimension.linux,
        targetDir: 'host_debug',
      );

      final et = _engineTool(RunCommand(
        environment: testEnvironment,
        configs: {
          'linux_test_config': builders.buildConfig(
            path: 'ci/builders/linux_test_config.json',
          ),
        },
        flutterTool: flutterTool,
      ));

      expect(
        () => et.run(['run', '--rbe', '--config=android_debug_arm64']),
        throwsA(
          isA<FatalError>().having(
            (a) => a.toString(),
            'toString()',
            contains('RBE was requested but no RBE config was found'),
          ),
        ),
      );
    });

    group('fails if -j is not a positive integer', () {
      for (final arg in ['-1', '0', 'foo']) {
        test('fails if -j is $arg', () async {
          final builders = TestBuilderConfig();
          builders.addBuild(
            name: 'linux/android_debug_arm64',
            dimension: TestDroneDimension.linux,
            targetDir: 'android_debug_arm64',
          );
          builders.addBuild(
            name: 'linux/host_debug',
            dimension: TestDroneDimension.linux,
            targetDir: 'host_debug',
          );

          final et = _engineTool(RunCommand(
            environment: testEnvironment,
            configs: {
              'linux_test_config': builders.buildConfig(
                path: 'ci/builders/linux_test_config.json',
              ),
            },
            flutterTool: flutterTool,
          ));

          expect(
            () => et.run([
              'run',
              '--config=android_debug_arm64',
              '--concurrency=$arg',
            ]),
            throwsA(
              isA<FatalError>().having(
                (a) => a.toString(),
                'toString()',
                contains('concurrency (-j) must specify a positive integer'),
              ),
            ),
          );
        });
      }
    });
  });
}

CommandRunner<int> _engineTool(RunCommand runCommand) {
  return CommandRunner<int>(
    'et',
    'Fake tool with a single instrumented command.',
  )..addCommand(runCommand);
}

Platform _fakePlatform(
  String os, {
  int numberOfProcessors = 32,
  String pathSeparator = '/',
}) {
  return FakePlatform(
    operatingSystem: os,
    resolvedExecutable: io.Platform.resolvedExecutable,
    numberOfProcessors: numberOfProcessors,
    pathSeparator: pathSeparator,
  );
}

final class _FakeFlutterTool implements FlutterTool {
  List<Device> respondWithDevices = [];

  @override
  Future<List<Device>> devices() async {
    return respondWithDevices;
  }
}
