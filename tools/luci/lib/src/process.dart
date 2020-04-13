// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:io' as io;

import 'package:meta/meta.dart';

import 'environment.dart';
import 'exceptions.dart';

/// There might be proccesses started during the tests.
///
/// Use this list to store those Processes, for cleaning up before shutdown.
final List<io.Process> processesToCleanUp = <io.Process>[];

/// Runs [executable] merging its output into the current process' standard out and standard error.
Future<int> runProcess(
  String executable,
  List<String> arguments, {
  String workingDirectory,
  Map<String, String> environment,
  bool mustSucceed = false,
}) async {
  environment ??= const <String, String>{};
  final io.Process process = await io.Process.start(
    executable,
    arguments,
    workingDirectory: workingDirectory,
    // Running the process in a system shell for Windows. Otherwise
    // the process is not able to get Dart from path.
    runInShell: io.Platform.isWindows,
    mode: io.ProcessStartMode.inheritStdio,
    environment: environment,
  );
  final int exitCode = await process.exitCode;
  if (mustSucceed && exitCode != 0) {
    throw ProcessException(
      description: 'Sub-process failed.',
      executable: executable,
      arguments: arguments,
      workingDirectory: workingDirectory,
      exitCode: exitCode,
    );
  }
  return exitCode;
}

/// Runs [executable]. Do not follow the exit code or the output.
void startProcess(
  String executable,
  List<String> arguments, {
  String workingDirectory,
  Map<String, String> environment,
  bool mustSucceed = false,
}) async {
  environment ??= const <String, String>{};
  final io.Process process = await io.Process.start(
    executable,
    arguments,
    workingDirectory: workingDirectory,
    // Running the process in a system shell for Windows. Otherwise
    // the process is not able to get Dart from path.
    runInShell: io.Platform.isWindows,
    mode: io.ProcessStartMode.inheritStdio,
    environment: environment,
  );
  processesToCleanUp.add(process);
}

/// Runs [executable] and returns its standard output as a string.
///
/// If the process fails, throws a [ProcessException].
Future<String> evalProcess(
  String executable,
  List<String> arguments, {
  String workingDirectory,
  Map<String, String> environment,
}) async {
  environment ??= const <String, String>{};
  final io.ProcessResult result = await io.Process.run(
    executable,
    arguments,
    workingDirectory: workingDirectory,
    environment: environment,
  );
  if (result.exitCode != 0) {
    throw ProcessException(
      description: result.stderr as String,
      executable: executable,
      arguments: arguments,
      workingDirectory: workingDirectory,
      exitCode: result.exitCode,
    );
  }
  return result.stdout as String;
}

/// Runs the `flutter` command using the local engine.
Future<void> runFlutter(
  String workingDirectory,
  List<String> arguments, {
  bool useSystemFlutter = false,
}) async {
  final String executable =
      useSystemFlutter ? 'flutter' : environment.flutterCommand.path;
  arguments.add('--local-engine=host_debug_unopt');
  final int exitCode = await runProcess(
    executable,
    arguments,
    workingDirectory: workingDirectory,
  );

  if (exitCode != 0) {
    throw ToolException('ERROR: Failed to run $executable with '
        'arguments ${arguments.toString()}. Exited with exit code $exitCode');
  }
}

/// Thrown by process utility functions, such as [evalProcess], when a process
/// exits with a non-zero exit code.
@immutable
class ProcessException implements Exception {
  /// Instantiates a process exception.
  const ProcessException({
    @required this.description,
    @required this.executable,
    @required this.arguments,
    @required this.workingDirectory,
    @required this.exitCode,
  });

  /// Describes what went wrong.
  final String description;

  /// The executable used to start the process that failed.
  final String executable;

  /// Arguments passed to the [executable].
  final List<String> arguments;

  /// The working directory of the process.
  final String workingDirectory;

  /// The exit code that the process exited with, if it exited.
  final int exitCode;

  @override
  String toString() {
    final StringBuffer message = StringBuffer();
    message
      ..writeln(description)
      ..writeln('Command: $executable ${arguments.join(' ')}')
      ..writeln(
          'Working directory: ${workingDirectory ?? io.Directory.current.path}')
      ..writeln('Exit code: $exitCode');
    return '$message';
  }
}
