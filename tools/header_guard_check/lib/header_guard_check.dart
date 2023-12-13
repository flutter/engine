// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:engine_repo_tools/engine_repo_tools.dart';
import 'package:meta/meta.dart';
import 'package:path/path.dart' as p;

import 'src/header_file.dart';

/// Checks C++ header files for header guards.
@immutable
final class HeaderGuardCheck {
  /// Creates a new header guard checker.
  const HeaderGuardCheck({
    required this.source,
    required this.exclude,
    this.include = const <String>[],
  });

  /// Parses the command line arguments and creates a new header guard checker.
  factory HeaderGuardCheck.fromCommandLine(List<String> arguments) {
    final ArgResults argResults = _parser.parse(arguments);
    return HeaderGuardCheck(
      source: Engine.fromSrcPath(argResults['root'] as String),
      include: argResults['include'] as List<String>,
      exclude: argResults['exclude'] as List<String>,
    );
  }

  /// Engine source root.
  final Engine source;

  /// Path directories to include in the check.
  final List<String> include;

  /// Path directories to exclude from the check.
  final List<String> exclude;

  /// Runs the header guard check.
  Future<int> run() async {
    final List<io.File> files = <io.File>[];

    // Recursive search for header files.
    final io.Directory dir = source.flutterDir;
    await for (final io.FileSystemEntity entity in dir.list(recursive: true)) {
      if (entity is io.File && entity.path.endsWith('.h')) {
        // Check that the file is included.
        bool included = include.isEmpty;
        for (final String includePath in include) {
          final String relativePath = p.relative(includePath, from: source.flutterDir.path);
          if (p.isWithin(relativePath, entity.path)) {
            included = true;
            break;
          }
        }
        if (!included) {
          continue;
        }

        // Check that the file is not excluded.
        bool excluded = false;
        for (final String excludePath in exclude) {
          final String relativePath = p.relative(excludePath, from: source.flutterDir.path);
          if (p.isWithin(relativePath, entity.path)) {
            excluded = true;
            break;
          }
        }
        if (!excluded) {
          files.add(entity);
        }
      }
    }

    // Check each file.
    final List<HeaderFile> badFiles = <HeaderFile>[];
    for (final io.File file in files) {
      final HeaderFile headerFile = HeaderFile.parse(file.path);
      if (headerFile.pragmaOnce != null) {
        io.stderr.writeln(headerFile.pragmaOnce!.message('Unexpected #pragma once'));
        badFiles.add(headerFile);
        continue;
      }
      if (headerFile.guard == null) {
        io.stderr.writeln('Missing header guard in ${headerFile.path}');
        badFiles.add(headerFile);
        continue;
      }

      // Determine what the expected guard should be.
      // Find the relative path from the engine root to the file.
      final String relativePath = p.relative(file.path, from: source.flutterDir.path);
      final String underscoredRelativePath = relativePath.replaceAll(p.separator, '_');
      final String expectedGuard = 'FLUTTER_${p.withoutExtension(underscoredRelativePath).toUpperCase().replaceAll('.', '_')}_H_';
      if (headerFile.guard!.ifndefValue != expectedGuard) {
        io.stderr.writeln(headerFile.guard!.ifndefSpan!.message('Expected #ifndef $expectedGuard'));
        badFiles.add(headerFile);
        continue;
      }
      if (headerFile.guard!.defineValue != expectedGuard) {
        io.stderr.writeln(headerFile.guard!.defineSpan!.message('Expected #define $expectedGuard'));
        badFiles.add(headerFile);
        continue;
      }
      if (headerFile.guard!.endifValue != expectedGuard) {
        io.stderr.writeln(headerFile.guard!.endifSpan!.message('Expected #endif // $expectedGuard'));
        badFiles.add(headerFile);
        continue;
      }
    }

    if (badFiles.isNotEmpty) {
      io.stdout.writeln('The following ${badFiles.length} files have invalid header guards:');
      for (final HeaderFile headerFile in badFiles) {
        io.stdout.writeln('  ${headerFile.path}');
      }
      return 1;
    }

    return 0;
  }
}

final Engine? _engine = Engine.tryFindWithin(p.dirname(p.fromUri(io.Platform.script)));

final ArgParser _parser = ArgParser()
  ..addOption(
    'root',
    abbr: 'r',
    help: 'Path to the engine source root.',
    valueHelp: 'path/to/engine/src',
    defaultsTo: _engine?.srcDir.path,
  )
  ..addMultiOption(
    'include',
    abbr: 'i',
    help: 'Path directories to include in the check.',
    valueHelp: 'path/to/dir (relative to the engine root)',
    defaultsTo: <String>[],
  )
  ..addMultiOption(
    'exclude',
    abbr: 'e',
    help: 'Path directories to exclude from the check.',
    valueHelp: 'path/to/dir (relative to the engine root)',
    defaultsTo: _engine != null ? <String>[
      'build',
      'prebuilts',
      'third_party',
    ] : null,
  );
