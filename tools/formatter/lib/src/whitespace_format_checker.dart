// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

// ignore: import_of_legacy_library_into_null_safe
import 'package:isolate/isolate.dart';
import 'package:meta/meta.dart';
import 'package:path/path.dart' as path;
import 'package:process_runner/process_runner.dart';
import 'package:process/process.dart';

import '../format_checker.dart';

@immutable
class _GrepResult {
  const _GrepResult(
    this.file, [
    this.hits = const <String>[],
    this.lineNumbers = const <int>[],
  ]);
  bool get isEmpty => hits.isEmpty && lineNumbers.isEmpty;
  final io.File file;
  final List<String> hits;
  final List<int> lineNumbers;
}

/// Checks for trailing whitspace in Dart files.
class WhitespaceFormatChecker extends FormatChecker {
  /// A [FormatChecker] that check for trailing whitespace in .dart files.
  WhitespaceFormatChecker({
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
    );

  @override
  Future<bool> checkFormatting() async {
    final List<io.File> failures = await _getWhitespaceFailures();
    return failures.isEmpty;
  }

  /// A retular expression that matches trailing whitespace.
  static final RegExp trailingWsRegEx = RegExp(r'[ \t]+$', multiLine: true);

  @override
  Future<bool> fixFormatting() async {
    final List<io.File> failures = await _getWhitespaceFailures();
    if (failures.isNotEmpty) {
      for (final io.File file in failures) {
        io.stderr.writeln('Fixing $file');
        String contents = file.readAsStringSync();
        contents = contents.replaceAll(trailingWsRegEx, '');
        file.writeAsStringSync(contents);
      }
    }
    return true;
  }

  static Future<_GrepResult> _hasTrailingWhitespace(io.File file) async {
    final List<String> hits = <String>[];
    final List<int> lineNumbers = <int>[];
    int lineNumber = 0;
    for (final String line in file.readAsLinesSync()) {
      if (trailingWsRegEx.hasMatch(line)) {
        hits.add(line);
        lineNumbers.add(lineNumber);
      }
      lineNumber++;
    }
    if (hits.isEmpty) {
      return _GrepResult(file);
    }
    return _GrepResult(file, hits, lineNumbers);
  }

  Stream<_GrepResult> _whereHasTrailingWhitespace(
    Iterable<io.File> files,
  ) async* {
    final LoadBalancer pool = await LoadBalancer.create(
      io.Platform.numberOfProcessors,
      IsolateRunner.spawn,
    );
    for (final io.File file in files) {
      yield await pool.run<_GrepResult, io.File>(_hasTrailingWhitespace, file);
    }
  }

  Future<List<io.File>> _getWhitespaceFailures() async {
    final List<String> files = await getFileList(<String>[
      '*.c',
      '*.cc',
      '*.cpp',
      '*.cxx',
      '*.dart',
      '*.gn',
      '*.gni',
      '*.gradle',
      '*.h',
      '*.java',
      '*.json',
      '*.m',
      '*.mm',
      '*.py',
      '*.sh',
      '*.yaml',
    ]);
    if (files.isEmpty) {
      message('No files that differ, skipping whitespace check.');
      return <io.File>[];
    }
    message(
      'Checking for trailing whitespace on ${files.length} source '
      'file${files.length > 1 ? 's' : ''}...',
    );

    final ProcessPoolProgressReporter reporter = namedReport('whitespace');
    final List<_GrepResult> found = <_GrepResult>[];
    final int total = files.length;
    int completed = 0;
    int inProgress = io.Platform.numberOfProcessors;
    int pending = total;
    int failed = 0;
    await for (final _GrepResult result in _whereHasTrailingWhitespace(
      files.map<io.File>(
        (String file) => io.File(
          path.join(repoDir.absolute.path, file),
        ),
      ),
    )) {
      if (result.isEmpty) {
        completed++;
      } else {
        failed++;
        found.add(result);
      }
      pending--;
      inProgress = pending < io.Platform.numberOfProcessors
        ? pending
        : io.Platform.numberOfProcessors;
      reporter(total, completed, inProgress, pending, failed);
    }
    reportDone();
    if (found.isNotEmpty) {
      error(
        'Whitespace check failed. The following files have trailing spaces:',
      );
      for (final _GrepResult result in found) {
        for (int i = 0; i < result.hits.length; ++i) {
          message(
            '  ${result.file.path}:${result.lineNumbers[i]}:${result.hits[i]}',
          );
        }
      }
    } else {
      message('No trailing whitespace found.');
    }
    return found.map<io.File>((_GrepResult result) => result.file).toList();
  }
}
