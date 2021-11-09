// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io show Directory, File, Platform, stderr;

import 'package:args/args.dart';

/// A class for organizing the options to the Engine linter, and the files
/// that it operates on.
class Options {
  /// Builds an instance of [Options] from the arguments.
  Options({
    required this.buildCommandsPaths,
    required this.repoPath,
    this.help = false,
    this.verbose = false,
    this.checksArg = '',
    this.lintAll = false,
    this.errorMessage,
    StringSink? errSink,
  }) {
    checks = checksArg.isNotEmpty ? '--checks=$checksArg' : '--config=';
    _errSink = errSink ?? io.stderr;
  }

  factory Options._error(
    String message, {
    StringSink? errSink,
  }) {
    return Options(
      errorMessage: message,
      buildCommandsPaths: <io.File>[],
      repoPath: io.Directory('none'),
      errSink: errSink,
    );
  }

  factory Options._help({
    StringSink? errSink,
  }) {
    return Options(
      help: true,
      buildCommandsPaths: <io.File>[],
      repoPath: io.Directory('none'),
      errSink: errSink,
    );
  }

  /// Builds an [Options] instance with an [ArgResults] instance.
  factory Options._fromArgResults(
    ArgResults options, {
    StringSink? errSink,
  }) {
    final List<String> compileCommandsList = options['compile-commands'] as List<String>? ?? <String>[];
    return Options(
      help: options['help'] as bool,
      verbose: options['verbose'] as bool,
      buildCommandsPaths: compileCommandsList.map((String path) => io.File(path)).toList(),
      repoPath: io.Directory(options['repo'] as String),
      checksArg: options.wasParsed('checks') ? options['checks'] as String : '',
      lintAll: io.Platform.environment['FLUTTER_LINT_ALL'] != null ||
               options['lint-all'] as bool,
      errSink: errSink,
    );
  }

  /// Builds an instance of [Options] from the given `arguments`.
  factory Options.fromCommandLine(
    List<String> arguments, {
    StringSink? errSink,
  }) {
    final ArgResults argResults = _argParser.parse(arguments);
    final String? message = _checkArguments(argResults);
    if (message != null) {
      return Options._error(message, errSink: errSink);
    }
    if (argResults['help'] as bool) {
      return Options._help(errSink: errSink);
    }
    return Options._fromArgResults(
      argResults,
      errSink: errSink,
    );
  }

  static final ArgParser _argParser = ArgParser()
    ..addFlag(
      'help',
      help: 'Print help.',
    )
    ..addFlag(
      'lint-all',
      help: 'lint all of the sources, regardless of FLUTTER_NOLINT.',
      defaultsTo: false,
    )
    ..addFlag(
      'verbose',
      help: 'Print verbose output.',
      defaultsTo: false,
    )
    ..addOption(
      'repo',
      help: 'Use the given path as the repo path',
    )
    ..addMultiOption(
      'compile-commands',
      valueHelp: 'path/to/src/out/host_debug/compile_commands.json,path/to/src/out/ios_debug/compile_commands.json',
      help: 'List of paths to compile_commands.json. This '
            'file is created by running tools/gn. '
            'Pass this option multiple times to pass multiple json files.',
    )
    ..addOption(
      'checks',
      help: 'Perform the given checks on the code. Defaults to the empty '
            'string, indicating all checks should be performed.',
      defaultsTo: '',
    );

  /// Whether to print a help message and exit.
  final bool help;

  /// Whether to run with verbose output.
  final bool verbose;

  /// Paths to the compile_commands.json files.
  final List<io.File> buildCommandsPaths;

  /// The root of the flutter/engine repository.
  final io.Directory repoPath;

  /// Arguments to plumb through to clang-tidy formatted as a command line
  /// argument.
  final String checksArg;

  /// Check arguments to plumb through to clang-tidy.
  late final String checks;

  /// Whether all files should be linted.
  final bool lintAll;

  /// If there was a problem with the command line arguments, this string
  /// contains the error message.
  final String? errorMessage;

  late final StringSink _errSink;

  /// Print command usage with an additional message.
  void printUsage({String? message}) {
    if (message != null) {
      _errSink.writeln(message);
    }
    _errSink.writeln(
      'Usage: bin/main.dart [--help] [--lint-all] [--verbose] [--diff-branch]',
    );
    _errSink.writeln(_argParser.usage);
  }

  /// Command line argument validation.
  static String? _checkArguments(ArgResults argResults) {
    if (argResults.wasParsed('help')) {
      return null;
    }

    if (!argResults.wasParsed('compile-commands')) {
      return 'ERROR: The --compile-commands argument is required.';
    }

    if (!argResults.wasParsed('repo')) {
      return 'ERROR: The --repo argument is required.';
    }

    final List<String> compileCommandsList = argResults['compile-commands'] as List<String>? ?? <String>[];
    for (final io.File buildCommandsPath in compileCommandsList.map((String path) => io.File(path))
        .where((io.File file) => !file.existsSync())) {
      return "ERROR: Build commands path ${buildCommandsPath.absolute.path} doesn't exist.";
    }

    final io.Directory repoPath = io.Directory(argResults['repo'] as String);
    if (!repoPath.existsSync()) {
      return "ERROR: Repo path ${repoPath.absolute.path} doesn't exist.";
    }

    return null;
  }
}
