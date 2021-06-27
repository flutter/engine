// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:path/path.dart' as path;
import 'package:process_runner/process_runner.dart';
import 'package:process/process.dart';

import '../format_checker.dart';

/// Checks the format of any BUILD.gn files using the "gn format" command.
class GnFormatChecker extends FormatChecker {
  /// A [FormatChecker] that checks GN (.gn, .gni) files.
  GnFormatChecker({
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
    gnBinary = io.File(
      path.join(
        repoDir.absolute.path,
        'third_party',
        'gn',
        io.Platform.isWindows ? 'gn.exe' : 'gn',
      ),
    );
  }

  /// A [io.File] object for the `gn` binary.
  late final io.File gnBinary;

  @override
  Future<bool> checkFormatting() async {
    message('Checking GN formatting...');
    return (await _runGnCheck(fixing: false)) == 0;
  }

  @override
  Future<bool> fixFormatting() async {
    message('Fixing GN formatting...');
    await _runGnCheck(fixing: true);
    // The GN script shouldn't fail when fixing errors.
    return true;
  }

  Future<int> _runGnCheck({required bool fixing}) async {
    final List<String> filesToCheck = await getFileList(
      <String>['*.gn', '*.gni'],
    );

    final List<String> cmd = <String>[
      gnBinary.path,
      'format',
      if (!fixing) '--dry-run',
    ];
    final List<WorkerJob> jobs = <WorkerJob>[];
    for (final String file in filesToCheck) {
      jobs.add(WorkerJob(<String>[...cmd, file]));
    }
    final ProcessPool gnPool = ProcessPool(
      processRunner: processRunner,
      printReport: namedReport('gn format'),
    );
    final List<WorkerJob> completedJobs = await gnPool.runToCompletion(jobs);
    reportDone();
    final List<String> incorrect = <String>[];
    for (final WorkerJob job in completedJobs) {
      if (job.result.exitCode == 2) {
        incorrect.add('  ${job.command.last}');
      }
      if (job.result.exitCode == 1) {
        // GN has exit code 1 if it had some problem formatting/checking the
        // file.
        throw FormattingException(
          'Unable to format ${job.command.last}:\n${job.result.output}',
        );
      }
    }
    if (incorrect.isNotEmpty) {
      final bool plural = incorrect.length > 1;
      if (fixing) {
        message(
          'Fixed ${incorrect.length} GN file${plural ? 's' : ''}'
          ' which ${plural ? 'were' : 'was'} formatted incorrectly.',
        );
      } else {
        error(
          'Found ${incorrect.length} GN file${plural ? 's' : ''}'
          ' which ${plural ? 'were' : 'was'} formatted incorrectly:',
        );
        incorrect.forEach(io.stderr.writeln);
      }
    } else {
      message('All GN files formatted correctly.');
    }
    return incorrect.length;
  }
}
