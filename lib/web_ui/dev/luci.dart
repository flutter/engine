// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:async';
import 'dart:convert' show JsonEncoder;
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:meta/meta.dart';

import 'environment.dart';

/// The list of available targets.
///
/// Using GN-esque/Bazel-esque format so if we ever move to one of those
/// there's not a lot of relearning to do, but even if we don't at least
/// we'd be using familiar concepts.
const List<Target> targets = <Target>[
  UnitTestsTarget(),
  IntegrationTestsTarget(),
  LicensesTarget(),
];

CommandRunner runner = CommandRunner<bool>(
  'luci',
  'Run CI targets for the Flutter Web Engine.',
)
  ..addCommand(TargetsCommand())
  ..addCommand(RunCommand());

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    // The felt tool was invoked with no arguments. Print usage.
    runner.printUsage();
    io.exit(64); // Exit code 64 indicates a usage error.
  }

  await runner.run(args);
}

class TargetsCommand extends Command<bool> {
  TargetsCommand() {
    argParser.addFlag('pretty', help: 'Prints in human-readable format.');
  }

  @override
  String get name => 'targets';

  @override
  String get description => 'Prints the list of all available targets in JSON format.';

  @override
  FutureOr<bool> run() async {
    final List<Map<String, dynamic>> targetListJson = <Map<String, dynamic>>[];
    for (final Target target in targets) {
      targetListJson.add(target.toJson());
    }

    final JsonEncoder encoder = argResults['pretty'] as bool
      ? const JsonEncoder.withIndent('  ')
      : const JsonEncoder();

    print(encoder.convert(targetListJson));
    return true;
  }
}

class RunCommand extends Command<bool> {
  RunCommand() {

  }

  @override
  String get name => 'run';

  @override
  String get description => 'Runs a build target.';

  List<String> get targetNames => argResults.rest;

  @override
  FutureOr<bool> run() async {
    for (final String targetName in targetNames) {
      final Target target = targets.singleWhere((Target t) => t.name == targetName, orElse: () {
        throw Exception('Target $targetName not found.');
      });
      await target.run();
    }
    return true;
  }
}

abstract class Target {
  const Target({@required this.name});

  final String name;

  Future<void> run();

  Map<String, dynamic> toJson() {
    return <String, dynamic>{
      'name': name,
    };
  }
}

class UnitTestsTarget extends Target {
  const UnitTestsTarget() : super(
    name: '//lib/web_ui:unit_tests',
  );

  @override
  Future<void> run() async {
    print('>>> Running unit-tests');

  }
}

class IntegrationTestsTarget extends Target {
  const IntegrationTestsTarget() : super(
    name: '//lib/web_ui:integration_tests',
  );

  @override
  Future<void> run() async {
    print('>>> Running unit-tests');
  }
}

class LicensesTarget extends Target {
  const LicensesTarget() : super(
    name: '//lib/web_ui:licenses',
  );

  @override
  Future<void> run() async {
    print('>>> Running license header check');
  }
}
