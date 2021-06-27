// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:meta/meta.dart';
import 'package:process_runner/process_runner.dart';
import 'package:process/process.dart';

import 'src/clang_format_checker.dart';
import 'src/gn_format_checker.dart';
import 'src/java_format_checker.dart';
import 'src/whitespace_format_checker.dart';

/// A formatting error.
class FormattingException implements Exception {
  /// A formatting error with the error message and optionally the result from
  /// the format checker subprocess.
  FormattingException(this.message, [this.result]);

  /// The formatting error message.
  final String message;

  /// The result of the formatting tool subprocess.
  final io.ProcessResult? result;

  /// The exit code of the formatting tool subprocess.
  int get exitCode => result?.exitCode ?? -1;

  @override
  String toString() {
    final StringBuffer output = StringBuffer(runtimeType.toString());
    output.write(': $message');
    final String stderr = result?.stderr! as String;
    if (stderr.isNotEmpty) {
      output.write(':\n$stderr');
    }
    return output.toString();
  }
}

/// The type of the message from this library.
enum MessageType {
  /// A regular, informative message.
  message,

  /// An error message.
  error,

  /// A warning message.
  warning,
}

/// The type of check that a file needs.
enum FormatCheck {
  /// Check with clang-format.
  clang,

  /// Check with the Java format checker.
  java,

  /// Check for trailing whitespace in Dart files.
  whitespace,

  /// Check with the GN format checker.
  gn,
}

/// Translates a [String] name to a [FormatCheck].
FormatCheck nameToFormatCheck(String name) {
  switch (name) {
    case 'clang':
      return FormatCheck.clang;
    case 'java':
      return FormatCheck.java;
    case 'whitespace':
      return FormatCheck.whitespace;
    case 'gn':
      return FormatCheck.gn;
    default:
      throw FormattingException('Unknown FormatCheck type $name');
  }
}

/// Translate a [FormatCheck] to a [String].
String formatCheckToName(FormatCheck check) {
  switch (check) {
    case FormatCheck.clang:
      return 'C++/ObjC';
    case FormatCheck.java:
      return 'Java';
    case FormatCheck.whitespace:
      return 'Trailing whitespace';
    case FormatCheck.gn:
      return 'GN';
  }
}

/// Compute the set of [FormatCheck]s that are allowed on a platform.
///
/// NB: On Windows only GN and whitespace are checked.
List<String> formatCheckNames() {
  List<FormatCheck> allowed;
  if (io.Platform.isWindows) {
    allowed = <FormatCheck>[FormatCheck.gn, FormatCheck.whitespace];
  } else {
    allowed = FormatCheck.values;
  }
  return allowed
    .map<String>(
      (FormatCheck check) => check.toString().replaceFirst('$FormatCheck.', ''),
    )
    .toList();
}

Future<String> _runGit(
  List<String> args,
  ProcessRunner processRunner, {
  bool failOk = false,
}) async {
  final ProcessRunnerResult result = await processRunner.runProcess(
    <String>['git', ...args],
    failOk: failOk,
  );
  return result.stdout;
}

/// Type alias for message callbacks.
typedef MessageCallback = Function(String? message, {MessageType type});

/// Base class for format checkers.
///
/// Provides services that all format checkers need.
abstract class FormatChecker {
  /// Makes a [FormatChecker].
  ///
  /// `processManager` - Runs the format checker subprocess and any other
  ///   auxialiary tools it needs.
  /// `repoDir` - The directory of the flutter/engine checkout.
  /// `srcDr` - The parent directory of the flutter/engine checkout.
  /// `allFiles` - Indicates that all formattable files should have their
  ///   format check.
  /// `verbose` - Whether verbose output should be generated.
  /// `messageCallback` - How to generate output.
  FormatChecker({
    ProcessManager processManager = const LocalProcessManager(),
    required this.repoDir,
    required this.srcDir,
    this.allFiles = false,
    this.verbose = false,
    this.messageCallback,
  }) :
    processRunner = ProcessRunner(
      defaultWorkingDirectory: repoDir,
      processManager: processManager,
    );

  /// Factory method that creates subclass format checkers based on the type of
  /// check.
  factory FormatChecker.ofType(
    FormatCheck check, {
    ProcessManager processManager = const LocalProcessManager(),
    required io.Directory repoDir,
    required io.Directory srcDir,
    bool allFiles = false,
    bool verbose = false,
    MessageCallback? messageCallback,
  }) {
    switch (check) {
      case FormatCheck.clang:
        return ClangFormatChecker(
          processManager: processManager,
          repoDir: repoDir,
          srcDir: srcDir,
          allFiles: allFiles,
          verbose: verbose,
          messageCallback: messageCallback,
        );
      case FormatCheck.java:
        return JavaFormatChecker(
          processManager: processManager,
          repoDir: repoDir,
          srcDir: srcDir,
          allFiles: allFiles,
          verbose: verbose,
          messageCallback: messageCallback,
        );
      case FormatCheck.whitespace:
        return WhitespaceFormatChecker(
          processManager: processManager,
          repoDir: repoDir,
          srcDir: srcDir,
          allFiles: allFiles,
          verbose: verbose,
          messageCallback: messageCallback,
        );
      case FormatCheck.gn:
        return GnFormatChecker(
          processManager: processManager,
          repoDir: repoDir,
          srcDir: srcDir,
          allFiles: allFiles,
          verbose: verbose,
          messageCallback: messageCallback,
        );
    }
  }

  /// Utility for running checker and auxiliary processes.
  @protected
  final ProcessRunner processRunner;

  /// The flutter/engine checkout.
  final io.Directory repoDir;

  /// The parent of [repoDir].
  final io.Directory srcDir;

  /// Whether all files should have their formatting checked.
  final bool allFiles;

  /// Whether verbose output should be generated.
  final bool verbose;

  /// How to generate output.
  MessageCallback? messageCallback;

  /// Override to provide format checking for a specific type.
  Future<bool> checkFormatting();

  /// Override to provide format fixing for a specific type.
  Future<bool> fixFormatting();

  /// Generates a regular, informative message.
  @protected
  void message(String? string) => messageCallback?.call(
    string,
    type: MessageType.message,
  );

  /// Generates an error message.
  @protected
  void error(String string) => messageCallback?.call(
    string,
    type: MessageType.error,
  );

  /// Runs `git` in the `processRunner`.
  @protected
  Future<String> runGit(List<String> args) async => _runGit(
    args,
    processRunner,
  );

  /// Converts a given raw string of code units to a stream that yields those
  /// code units.
  ///
  /// Uses to convert the stdout of a previous command into an input stream for
  /// the next command.
  @protected
  Stream<List<int>> codeUnitsAsStream(List<int>? input) async* {
    if (input != null) {
      yield input;
    }
  }

  /// Uses `patch` to apply the fixes listed in `patches`.
  @protected
  Future<bool> applyPatch(List<String> patches) async {
    final ProcessPool patchPool = ProcessPool(
      processRunner: processRunner,
      printReport: namedReport('patch'),
    );
    final List<WorkerJob> jobs = patches.map<WorkerJob>((String patch) {
      return WorkerJob(
        <String>['patch', '-p0'],
        stdinRaw: codeUnitsAsStream(patch.codeUnits),
        failOk: true,
      );
    }).toList();
    final List<WorkerJob> completedJobs = await patchPool.runToCompletion(jobs);
    if (patchPool.failedJobs != 0) {
      error(
        '${patchPool.failedJobs} patch${patchPool.failedJobs > 1 ? 'es' : ''} '
        'failed to apply.',
      );
      completedJobs
        .where((WorkerJob job) => job.result.exitCode != 0)
        .map<String>((WorkerJob job) => job.result.output)
        .forEach(message);
    }
    return patchPool.failedJobs == 0;
  }

  Future<String> _getDiffBaseRevision() async {
    String upstream = 'upstream';
    final String upstreamUrl = await _runGit(
      <String>['remote', 'get-url', upstream],
      processRunner,
      failOk: true,
    );
    if (upstreamUrl.isEmpty) {
      upstream = 'origin';
    }
    await _runGit(<String>['fetch', upstream, 'master'], processRunner);
    String result = '';
    try {
      // This is the preferred command to use, but developer checkouts often do
      // not have a clear fork point, so we fall back to just the regular
      // merge-base in that case.
      result = await _runGit(
        <String>['merge-base', '--fork-point', 'FETCH_HEAD', 'HEAD'],
        processRunner,
      );
    } on ProcessRunnerException {
      result = await _runGit(
        <String>['merge-base', 'FETCH_HEAD', 'HEAD'],
        processRunner,
      );
    }
    return result.trim();
  }

  /// Gets the list of files to operate on.
  ///
  /// If [allFiles] is true, then returns all git controlled files in the repo
  /// of the given types.
  ///
  /// If [allFiles] is false, then only return those files of the given types
  /// that have changed between the current working tree and the [baseGitRef].
  @protected
  Future<List<String>> getFileList(List<String> types) async {
    final String baseGitRef = await _getDiffBaseRevision();
    String output;
    if (allFiles) {
      output = await runGit(<String>[
        'ls-files',
        '--',
        ...types,
      ]);
    } else {
      output = await runGit(<String>[
        'diff',
        '-U0',
        '--no-color',
        '--diff-filter=d',
        '--name-only',
        baseGitRef,
        '--',
        ...types,
      ]);
    }
    return output.split('\n').where((String line) => line.isNotEmpty).toList();
  }

  /// Generates a reporting function to supply to ProcessRunner to use instead
  /// of the default reporting function.
  @protected
  ProcessPoolProgressReporter namedReport(String name) {
    return (int total, int completed, int inProgress, int pending, int failed) {
      final String percent = total == 0
        ? '100'
        : ((100 * completed) ~/ total).toString().padLeft(3);
      final String completedStr = completed.toString().padLeft(3);
      final String totalStr = total.toString().padRight(3);
      final String inProgressStr = inProgress.toString().padLeft(2);
      final String pendingStr = pending.toString().padLeft(3);
      final String failedStr = failed.toString().padLeft(3);

      io.stderr.write(
        '$name Jobs: $percent% done, '
        '$completedStr/$totalStr completed, '
        '$inProgressStr in progress, '
        '$pendingStr pending, '
        '$failedStr failed.${' ' * 20}\r',
      );
    };
  }

  /// Clears the last printed report line so garbage isn't left on the terminal.
  @protected
  void reportDone() {
    io.stderr.write('\r${' ' * 100}\r');
  }
}
