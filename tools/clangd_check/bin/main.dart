// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:engine_repo_tools/engine_repo_tools.dart';
import 'package:path/path.dart' as p;

void main(List<String> args) {
  final Engine? engine = Engine.tryFindWithin();
  final ArgParser parser = ArgParser()
    ..addOption(
      'clangd',
      help: 'Path to clangd. Defaults to deriving the path from compile_commands.json.',
    )
    ..addOption(
      'compile-commands-dir',
      help: 'Path to a directory containing compile_commands.json.',
      defaultsTo: engine?.latestOutput()?.compileCommandsJson.parent.path,
    );
  final ArgResults results = parser.parse(args);

  final String? compileCommandsDir = results['compile-commands-dir'] as String?;
  if (compileCommandsDir == null) {
    io.stderr.writeln('Must provide a path to compile_commands.json');
    io.exitCode = 1;
    return;
  }
  final io.File compileCommandsFile = io.File(p.join(compileCommandsDir, 'compile_commands.json'));
  if (!compileCommandsFile.existsSync()) {
    io.stderr.writeln('No compile_commands.json found in $compileCommandsDir');
    io.exitCode = 1;
    return;
  }

  final List<Object?> compileCommands = json.decode(compileCommandsFile.readAsStringSync()) as List<Object?>;
  if (compileCommands.isEmpty) {
    io.stderr.writeln('Unexpected: compile_commands.json is empty');
    io.exitCode = 1;
    return;
  }

  String? clangd = results['clangd'] as String?;
  final Map<String, Object?> entry = compileCommands.first! as Map<String, Object?>;
  final String checkFile;
  if (entry case {
    'command': final String path,
    'file': final String file,
  }) {
    // Given a path like ../../flutter/foo.cc, we want to check foo.cc.
    checkFile = p.split(file).skip(3).join(p.separator);
    clangd ??= p.join(p.dirname(p.dirname(path.split(' ').first)), 'clang', 'bin', 'clangd');
  } else {
    io.stderr.writeln('Unexpected: compile_commands.json has an unexpected format');
    io.stderr.writeln('First entry: ${const JsonEncoder.withIndent('  ').convert(entry)}');
    io.exitCode = 1;
    return;
  }

  // Run clangd.
  try {
    final io.ProcessResult result = io.Process.runSync(clangd, <String>[
      '--compile-commands-dir',
      compileCommandsDir,
      '--check=$checkFile',
    ]);
    io.stdout.write(result.stdout);
    io.stderr.write(result.stderr);
    if ((result.stderr as String).contains('Path specified by --compile-commands-dir does not exist')) {
      io.stdout.writeln('clangd_check failed: --compile-commands-dir does not exist');
      io.exitCode = 1;
    } else if ((result.stderr as String).contains('Failed to resolve path')) {
      io.stdout.writeln('clangd_check failed: --check file does not exist');
      io.exitCode = 1;
    } else {
      io.exitCode = result.exitCode;
    }
  } on io.ProcessException catch (e) {
    io.stderr.writeln('Failed to run clangd: $e');
    io.stderr.writeln(const JsonEncoder.withIndent('  ').convert(entry));
    io.exitCode = 1;
  }
}
