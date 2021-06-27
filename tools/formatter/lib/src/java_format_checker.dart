// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:path/path.dart' as path;
import 'package:process_runner/process_runner.dart';
import 'package:process/process.dart';

import '../format_checker.dart';

/// Checks the format of Java files uing the Google Java format checker.
class JavaFormatChecker extends FormatChecker {
  /// A [FormatChecker] for Java files.
  JavaFormatChecker({
    ProcessManager processManager = const LocalProcessManager(),
    required io.Directory repoDir,
    required io.Directory srcDir,
    bool allFiles = false,
    bool verbose = false,
    MessageCallback? messageCallback,
  }) :
    super(
      processManager: processManager,
      repoDir: repoDir,
      srcDir: srcDir,
      allFiles: allFiles,
      verbose: verbose,
      messageCallback: messageCallback,
    )
  {
    googleJavaFormatJar = io.File(
      path.absolute(
        path.join(
          srcDir.absolute.path,
          'third_party',
          'android_tools',
          'google-java-format',
          'google-java-format-1.7-all-deps.jar',
        ),
      ),
    );
  }

  /// A [io.File] object for the Java formatter jar.
  late final io.File googleJavaFormatJar;

  Future<String> _getGoogleJavaFormatVersion() async {
    final ProcessRunnerResult result = await processRunner.runProcess(
      <String>['java', '-jar', googleJavaFormatJar.path, '--version'],
    );
    return result.stderr.trim();
  }

  @override
  Future<bool> checkFormatting() async {
    final List<String> failures = await _getJavaFormatFailures();
    failures.map(io.stdout.writeln);
    return failures.isEmpty;
  }

  @override
  Future<bool> fixFormatting() async {
    message('Fixing Java formatting...');
    final List<String> failures = await _getJavaFormatFailures(fixing: true);
    if (failures.isEmpty) {
      return true;
    }
    return applyPatch(failures);
  }

  Future<String> _getJavaVersion() async {
    final ProcessRunnerResult result = await processRunner.runProcess(
      <String>['java', '-version'],
    );
    return result.stderr.trim().split('\n')[0];
  }

  Future<List<String>> _getJavaFormatFailures({bool fixing = false}) async {
    message('Checking Java formatting...');
    final List<WorkerJob> formatJobs = <WorkerJob>[];
    final List<String> files = await getFileList(<String>['*.java']);
    if (files.isEmpty) {
      message('No Java files with changes, skipping Java format check.');
      return <String>[];
    }
    String javaVersion = '<unknown>';
    String javaFormatVersion = '<unknown>';
    try {
      javaVersion = await _getJavaVersion();
    } on ProcessRunnerException {
      error('Cannot run Java, skipping Java file formatting!');
      return const <String>[];
    }
    try {
      javaFormatVersion = await _getGoogleJavaFormatVersion();
    } on ProcessRunnerException {
      error('Cannot find google-java-format, skipping Java format check.');
      return const <String>[];
    }
    if (verbose) {
      message('Using $javaFormatVersion with Java $javaVersion');
    }
    for (final String file in files) {
      if (file.trim().isEmpty) {
        continue;
      }
      formatJobs.add(
        WorkerJob(
          <String>['java', '-jar', googleJavaFormatJar.path, file.trim()],
        ),
      );
    }
    final ProcessPool formatPool = ProcessPool(
      processRunner: processRunner,
      printReport: namedReport('Java format'),
    );
    final Stream<WorkerJob> completedClangFormats = formatPool.startWorkers(
      formatJobs,
    );
    final List<WorkerJob> diffJobs = <WorkerJob>[];
    await for (final WorkerJob completedJob in completedClangFormats) {
      if (completedJob.result.exitCode == 0) {
        diffJobs.add(
          WorkerJob(
            <String>['diff', '-u', completedJob.command.last, '-'],
            stdinRaw: codeUnitsAsStream(completedJob.result.stdoutRaw),
            failOk: true,
          ),
        );
      }
    }
    final ProcessPool diffPool = ProcessPool(
      processRunner: processRunner,
      printReport: namedReport('diff'),
    );
    final List<WorkerJob> completedDiffs = await diffPool.runToCompletion(
      diffJobs,
    );
    final Iterable<WorkerJob> failed = completedDiffs.where((WorkerJob job) {
      return job.result.exitCode != 0;
    });
    reportDone();
    if (failed.isNotEmpty) {
      final bool plural = failed.length > 1;
      if (fixing) {
        error(
          'Fixing ${failed.length} Java file${plural ? 's' : ''}'
          ' which ${plural ? 'were' : 'was'} formatted incorrectly.',
        );
      } else {
        error(
          'Found ${failed.length} Java file${plural ? 's' : ''}'
          ' which ${plural ? 'were' : 'was'} formatted incorrectly.',
        );
        for (final WorkerJob job in failed) {
          io.stdout.write(job.result.stdout);
        }
      }
    } else {
      message(
        'Completed checking ${diffJobs.length} Java files with no '
        'formatting problems.',
      );
    }
    return failed.map<String>((WorkerJob job) {
      return job.result.stdout;
    }).toList();
  }
}
