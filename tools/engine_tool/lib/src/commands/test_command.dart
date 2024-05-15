// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:engine_build_configs/engine_build_configs.dart';

import '../build_utils.dart';
import '../gn_utils.dart';
import '../label.dart';
import '../proc_utils.dart';
import '../worker_pool.dart';
import 'command.dart';
import 'flags.dart';

/// The root 'test' command.
final class TestCommand extends CommandBase {
  /// Constructs the 'test' command.
  TestCommand({
    required super.environment,
    required Map<String, BuilderConfig> configs,
    super.help = false,
    super.usageLineLength,
  }) {
    // When printing the help/usage for this command, only list all builds
    // when the --verbose flag is supplied.
    final bool includeCiBuilds = environment.verbose || !help;
    builds = runnableBuilds(environment, configs, includeCiBuilds);
    debugCheckBuilds(builds);
    addConfigOption(
      environment,
      argParser,
      builds,
    );
    argParser.addFlag(
      rbeFlag,
      defaultsTo: environment.hasRbeConfigInTree(),
      help: 'RBE is enabled by default when available.',
    );
  }

  /// List of compatible builds.
  late final List<Build> builds;

  @override
  String get name => 'test';

  @override
  String get description => '''
Runs a test target
et test //flutter/fml/...             # Run all test targets in `//flutter/fml` and its subdirectories.
et test //flutter/fml:all             # Run all test targets in `//flutter/fml`.
et test //flutter/fml:fml_benchmarks  # Run a single test target in `//flutter/fml`.
''';

  @override
  Future<int> run() async {
    final String configName = argResults![configFlag] as String;
    final bool useRbe = argResults![rbeFlag] as bool;
    if (useRbe && !environment.hasRbeConfigInTree()) {
      environment.logger.error('RBE was requested but no RBE config was found');
      return 1;
    }
    final String demangledName = demangleConfigName(environment, configName);
    final Build? build =
        builds.where((Build build) => build.name == demangledName).firstOrNull;
    if (build == null) {
      environment.logger.error('Could not find config $configName');
      return 1;
    }

    final List<BuildTarget>? selectedTargets = await targetsFromCommandLine(
      environment,
      build,
      argResults!.rest,
      defaultToAll: true,
      enableRbe: useRbe,
    );
    if (selectedTargets == null) {
      // The user typed something wrong and targetsFromCommandLine has already
      // logged the error message.
      return 1;
    }
    if (selectedTargets.isEmpty) {
      environment.logger.fatal(
        'targetsFromCommandLine unexpectedly returned an empty list',
      );
    }

    final List<BuildTarget> testTargets = <BuildTarget>[];
    for (final BuildTarget target in selectedTargets) {
      if (_isTestExecutable(target)) {
        testTargets.add(target);
      }
      if (target.executable == null) {
        environment.logger.fatal(
            '$target is an executable but is missing the executable path');
      }
    }

    final int buildExitCode = await runBuild(
      environment,
      build,
      targets: testTargets.map((BuildTarget target) => Label.parse(target.label)).toList(),
      enableRbe: useRbe,
    );
    if (buildExitCode != 0) {
      return buildExitCode;
    }
    final WorkerPool workerPool =
        WorkerPool(environment, ProcessTaskProgressReporter(environment));
    final Set<ProcessTask> tasks = <ProcessTask>{};
    for (final BuildTarget target in testTargets) {
      final List<String> commandLine = <String>[target.executable!.path];
      tasks.add(ProcessTask(
          target.label, environment, environment.engine.srcDir, commandLine));
    }
    return await workerPool.run(tasks) ? 0 : 1;
  }

  /// Returns true if `target` is a testonly executable.
  static bool _isTestExecutable(BuildTarget target) {
    return target.testOnly && target.type == BuildTargetType.executable;
  }
}
