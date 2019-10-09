// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:path/path.dart' as path;

import 'environment.dart';

class CleanCommand extends Command<bool> {
  CleanCommand() {
    argParser
      ..addFlag(
        'ninja',
        defaultsTo: false,
        help: 'Also clean up the engine out directory with ninja output.',
      );
  }

  @override
  String get name => 'clean';

  bool get _alsoCleanNinja => argResults['ninja'];

  @override
  String get description => 'Deletes build caches and artifacts.';

  @override
  FutureOr<bool> run() async {
    final io.File packagesFile = io.File(path.join(
      environment.webUiRootDir.path, '.packages'
    ));
    final io.File pubspecLockFile = io.File(path.join(
      environment.webUiRootDir.path, 'pubspec.lock'
    ));
    final io.Directory assetsDir = io.Directory(path.join(
      environment.webUiRootDir.path, 'lib', 'assets'
    ));
    final Iterable<io.File> fontFiles = assetsDir
      .listSync()
      .whereType<io.File>()
      .where((io.File file) => file.path.endsWith('.ttf'));

    await Future.wait(<Future<dynamic>>[
      if (environment.webUiDartToolDir.existsSync())
        environment.webUiDartToolDir.delete(recursive: true),
      if (environment.webUiBuildDir.existsSync())
        environment.webUiBuildDir.delete(recursive: true),
      if (packagesFile.existsSync())
        packagesFile.delete(),
      if (pubspecLockFile.existsSync())
        pubspecLockFile.delete(),
      for (io.File fontFile in fontFiles)
        fontFile.delete(),
      if (_alsoCleanNinja)
        environment.outDir.delete(recursive: true),
    ]);
    return true;
  }
}
