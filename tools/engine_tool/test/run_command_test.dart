// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show jsonDecode;
import 'dart:ffi';
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:engine_build_configs/engine_build_configs.dart';
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

import 'fixtures.dart' as fixtures;

void main() {
  final linuxTestConfig = BuilderConfig.fromJson(
    path: 'ci/builders/linux_test_config.json',
    map: jsonDecode(fixtures.testConfig('Linux', Platform.linux))
        as Map<String, Object?>,
  );

  final macTestConfig = BuilderConfig.fromJson(
    path: 'ci/builders/mac_test_config.json',
    map: jsonDecode(fixtures.testConfig('Mac-12', Platform.macOS))
        as Map<String, Object?>,
  );

  final winTestConfig = BuilderConfig.fromJson(
    path: 'ci/builders/win_test_config.json',
    map: jsonDecode(fixtures.testConfig('Windows-11', Platform.windows))
        as Map<String, Object?>,
  );

  final allTestConfigs = <String, BuilderConfig>{
    'linux_test_config': linuxTestConfig,
    'mac_test_config': macTestConfig,
    'win_test_config': winTestConfig,
  };

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
      abi: Abi.current(),
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
        abi: Abi.current(),
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
      final et = _engineTool(RunCommand(
        environment: testEnvironment,
        configs: allTestConfigs,
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
      final et = _engineTool(RunCommand(
        environment: testEnvironment,
        configs: allTestConfigs,
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
