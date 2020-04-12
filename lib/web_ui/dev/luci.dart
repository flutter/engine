// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// The command-line tool used to inspect and run LUCI build targets.

// @dart = 2.6
import 'dart:async';
import 'dart:convert' show JsonEncoder;
import 'dart:io' as io;

import 'package:args/command_runner.dart';

import 'luci_framework.dart';
import 'luci_targets.dart';

CommandRunner runner = CommandRunner<bool>(
  'luci',
  'Run CI targets for the Flutter Web Engine.',
)
  ..addCommand(TargetsCommand())
  ..addCommand(RunCommand());

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    // Invoked with no arguments. Print usage.
    runner.printUsage();
    io.exit(64); // Exit code 64 indicates a usage error.
  }

  try {
    await runner.run(args);
  } on LuciException catch(error) {
    io.stderr.writeln(error.message);
    io.exit(1);
  }
}

/// Prints available targets to the standard output in JSON format.
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

    print(encoder.convert(<String, dynamic>{
      'targets': targetListJson,
    }));
    return true;
  }
}

/// Runs LUCI targets.
class RunCommand extends Command<bool> {
  @override
  String get name => 'run';

  @override
  String get description => 'Runs targets.';

  List<String> get targetNames => argResults.rest;

  @override
  FutureOr<bool> run() async {
    for (final String targetName in targetNames) {
      final Target target = targets.singleWhere((Target t) => t.name == targetName, orElse: () {
        throw LuciException('Target $targetName not found.');
      });
      await target.runner.run(target);
    }
    return true;
  }
}

class LuciException implements Exception {
  LuciException(this.message);

  final String message;

  @override
  String toString() => '$LuciException: $message';
}
