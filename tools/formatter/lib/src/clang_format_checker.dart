// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:path/path.dart' as path;
import 'package:process_runner/process_runner.dart';
import 'package:process/process.dart';

import '../format_checker.dart';

/// Checks and formats C++/ObjC files using clang-format.
class ClangFormatChecker extends FormatChecker {
  /// Creates a [FormatChecker] that checks C++ and ObjC files.
  ClangFormatChecker({
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
    /*late*/ String clangOs;
    if (io.Platform.isLinux) {
      clangOs = 'linux-x64';
    } else if (io.Platform.isMacOS) {
      clangOs = 'mac-x64';
    } else {
      throw FormattingException(
        "Unknown operating system: don't know how to run clang-format here.",
      );
    }
    clangFormat = io.File(
      path.join(
        srcDir.absolute.path,
        'buildtools',
        clangOs,
        'clang',
        'bin',
        'clang-format',
      ),
    );
  }

  /// A [io.File] object for the `clang-format` binary.
  late final io.File clangFormat;

  @override
  Future<bool> checkFormatting() async {
    final List<String> failures = await _getCFormatFailures();
    failures.map(io.stdout.writeln);
    return failures.isEmpty;
  }

  @override
  Future<bool> fixFormatting() async {
    message('Fixing C++/ObjC formatting...');
    final List<String> failures = await _getCFormatFailures(fixing: true);
    if (failures.isEmpty) {
      return true;
    }
    return applyPatch(failures);
  }

  Future<String> _getClangFormatVersion() async {
    final ProcessRunnerResult result = await processRunner.runProcess(
      <String>[clangFormat.path, '--version'],
    );
    return result.stdout.trim();
  }

  Future<List<String>> _getCFormatFailures({bool fixing = false}) async {
    message('Checking C++/ObjC formatting...');
    const List<String> clangFiletypes = <String>[
      '*.c',
      '*.cc',
      '*.cxx',
      '*.cpp',
      '*.h',
      '*.m',
      '*.mm',
    ];
    final List<String> files = await getFileList(clangFiletypes);
    if (files.isEmpty) {
      message(
        'No C++/ObjC files with changes, skipping C++/ObjC format check.',
      );
      return <String>[];
    }
    if (verbose) {
      message('Using ${await _getClangFormatVersion()}');
    }
    final List<WorkerJob> clangJobs = <WorkerJob>[];
    for (final String file in files) {
      if (file.trim().isEmpty) {
        continue;
      }
      clangJobs.add(WorkerJob(<String>[
        clangFormat.path,
        '--style=file',
        file.trim(),
      ]));
    }
    final ProcessPool clangPool = ProcessPool(
      processRunner: processRunner,
      printReport: namedReport('clang-format'),
    );
    final Stream<WorkerJob> completedClangFormats = clangPool.startWorkers(
      clangJobs,
    );
    final List<WorkerJob> diffJobs = <WorkerJob>[];
    await for (final WorkerJob completedJob in completedClangFormats) {
      if (completedJob.result.exitCode == 0) {
        diffJobs.add(WorkerJob(
          <String>['diff', '-u', completedJob.command.last, '-'],
          stdinRaw: codeUnitsAsStream(completedJob.result.stdoutRaw),
          failOk: true,
        ));
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
        message('Fixing ${failed.length} C++/ObjC file${plural ? 's' : ''}'
            ' which ${plural ? 'were' : 'was'} formatted incorrectly.');
      } else {
        error('Found ${failed.length} C++/ObjC file${plural ? 's' : ''}'
            ' which ${plural ? 'were' : 'was'} formatted incorrectly.');
        for (final WorkerJob job in failed) {
          io.stdout.write(job.result.stdout);
        }
      }
    } else {
      message(
        'Completed checking ${diffJobs.length} C++/ObjC files with no '
        'formatting problems.',
      );
    }
    return failed.map<String>((WorkerJob job) {
      return job.result.stdout;
    }).toList();
  }
}
