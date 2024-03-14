// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:engine_build_configs/engine_build_configs.dart';
import 'package:path/path.dart' as p;

import '../build_utils.dart';
import '../gn_utils.dart';
import '../proc_utils.dart';
import '../worker_pool.dart';
import 'command.dart';
import 'flags.dart';

/// The root 'test' command.
final class TestCommand extends CommandBase {
  /// Constructs the 'build' command.
  TestCommand({
    required super.environment,
    required Map<String, BuilderConfig> configs,
  }) {
    builds = runnableBuilds(environment, configs);
    debugCheckBuilds(builds);
    argParser.addOption(
      configFlag,
      abbr: 'c',
      defaultsTo: 'host_debug',
      help: 'Specify the build config to use',
      allowed: <String>[
        for (final Build config in runnableBuilds(environment, configs))
          config.name,
      ],
      allowedHelp: <String, String>{
        for (final Build config in runnableBuilds(environment, configs))
          config.name: config.gn.join(' '),
      },
    );
  }

  /// List of compatible builds.
  late final List<Build> builds;

  @override
  String get name => 'test';

  @override
  String get description => 'Runs a test target';

  @override
  Future<int> run() async {
    final String configName = argResults![configFlag] as String;
    final Build? build =
        builds.where((Build build) => build.name == configName).firstOrNull;
    if (build == null) {
      environment.logger.error('Could not find config $configName');
      return 1;
    }

    final Map<String, TestTarget> allTargets = await findTestTargets(
        environment,
        Directory(p.join(environment.engine.outDir.path, build.ninja.config)));
    final Set<TestTarget> selectedTargets =
        selectTargets(argResults!.rest, allTargets);
    final List<String> buildTargets = selectedTargets
        .map<String>((TestTarget target) => target.label.substring(2))
        .toList();
    // TODO(johnmccutchan): runBuild manipulates buildTargets and adds some
    // targets listed in Build. Fix this.
    final int buildExitCode = await runBuild(environment, build, buildTargets);
    if (buildExitCode != 0) {
      return buildExitCode;
    }
    final WorkerPool workerPool =
        WorkerPool(environment, ProcessTaskProgressReporter(environment));
    final Set<ProcessTask> tasks = <ProcessTask>{};
    for (final TestTarget target in selectedTargets) {
      final List<String> commandLine = <String>[target.executable.path];
      tasks.add(ProcessTask(
          target.label, environment, environment.engine.srcDir, commandLine));
    }
    return await workerPool.run(tasks) ? 0 : 1;
  }
}
