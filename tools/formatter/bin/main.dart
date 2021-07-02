// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Checks and fixes format on files with changes.
//
// Run with --help for usage.

// TODO(gspencergoog): Support clang formatting on Windows.
// TODO(gspencergoog): Support Java formatting on Windows.

import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:formatter/format_checker.dart';
import 'package:process/process.dart';

// 1. Move the non CLI parts of this code to lib/
// 2. Test.

void _usage(ArgParser parser, {int exitCode = 1}) {
  io.stderr.writeln(
    'format.dart [--help] [--fix] [--all-files] '
    '[--check <${formatCheckNames().join('|')}>]',
  );
  io.stderr.writeln(parser.usage);
  io.exit(exitCode);
}

Future<int> main(List<String> arguments) async {
  final ArgParser parser = ArgParser()
    ..addFlag(
      'help',
      abbr: 'h',
      help: 'Print help.',
    )
    ..addFlag(
      'fix',
      abbr: 'f',
      help: 'Instead of just checking for formatting errors, '
            'fix them in place.',
      defaultsTo: false,
    )
    ..addFlag(
      'all-files',
      abbr: 'a',
      help: 'Instead of just checking for formatting errors in changed files, '
            'check for them in all files.',
      defaultsTo: false,
    )
    ..addMultiOption(
      'check',
      abbr: 'c',
      allowed: formatCheckNames(),
      defaultsTo: formatCheckNames(),
      help: 'Specifies which checks will be performed. Defaults to all checks. '
            'May be specified more than once to perform multiple types of '
            'checks. On Windows, only whitespace and gn checks are currently '
            'supported.',
    )
    ..addFlag(
      'verbose',
      help: 'Print verbose output.',
      defaultsTo: false,
    );

  late final ArgResults options;
  try {
    options = parser.parse(arguments);
  } on FormatException catch (e) {
    io.stderr.writeln('ERROR: $e');
    _usage(parser, exitCode: 0);
  }

  final bool verbose = options['verbose'] as bool;

  if (options['help'] as bool) {
    _usage(parser, exitCode: 0);
  }

  final io.File script = io.File.fromUri(io.Platform.script).absolute;
  final io.Directory repoDir = script.parent.parent.parent.parent;
  final io.Directory srcDir = repoDir.parent;
  if (verbose) {
    io.stderr.writeln('Repo: $repoDir');
    io.stderr.writeln('Src: $srcDir');
  }

  void message(String? message, {MessageType type = MessageType.message}) {
    message ??= '';
    switch (type) {
      case MessageType.message:
        io.stderr.writeln(message);
        break;
      case MessageType.error:
        io.stderr.writeln('ERROR: $message');
        break;
      case MessageType.warning:
        io.stderr.writeln('WARNING: $message');
        break;
    }
  }

  const ProcessManager processManager = LocalProcessManager();

  bool result = true;
  final List<String> checks = options['check'] as List<String>;
  try {
    for (final String checkName in checks) {
      final FormatCheck check = nameToFormatCheck(checkName);
      final String humanCheckName = formatCheckToName(check);
      final FormatChecker checker = FormatChecker.ofType(check,
        processManager: processManager,
        repoDir: repoDir,
        srcDir: srcDir,
        allFiles: options['all-files'] as bool,
        verbose: verbose,
        messageCallback: message,
      );
      bool stepResult;
      if (options['fix'] as bool) {
        message('Fixing any $humanCheckName format problems');
        stepResult = await checker.fixFormatting();
        if (!stepResult) {
          message('Unable to apply $humanCheckName format fixes.');
        }
      } else {
        message('Performing $humanCheckName format check');
        stepResult = await checker.checkFormatting();
        if (!stepResult) {
          message('Found $humanCheckName format problems.');
        }
      }
      result = result && stepResult;
    }
  } on FormattingException catch (e) {
    message('ERROR: $e', type: MessageType.error);
  }

  io.exit(result ? 0 : 1);
}
