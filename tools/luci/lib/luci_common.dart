// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:path/path.dart' as pathlib;

import 'src/environment.dart';
import 'src/process.dart';

export 'src/environment.dart';
export 'src/exceptions.dart';
export 'src/process.dart';

/// Adds utility methods to [Command] classes for parsing arguments.
mixin ArgUtils<T> on Command<T> {
  /// Extracts a boolean argument from [argResults].
  bool boolArg(String name) => argResults[name] as bool;

  /// Extracts a string argument from [argResults].
  String stringArg(String name) => argResults[name] as String;

  /// Extracts a integer argument from [argResults].
  ///
  /// If the argument value cannot be parsed as [int] throws an [ArgumentError].
  int intArg(String name) {
    final String rawValue = stringArg(name);
    if (rawValue == null) {
      return null;
    }
    final int value = int.tryParse(rawValue);
    if (value == null) {
      throw ArgumentError(
        'Argument $name should be an integer value but was "$rawValue"',
      );
    }
    return value;
  }
}

/// There might be temporary directories created during the tests.
///
/// Use this list to store those directories and for deleteing them before
/// shutdown.
final List<io.Directory> temporaryDirectories = <io.Directory>[];

/// A function that performs asynchronous work.
typedef AsyncCallback = Future<void> Function();

/// There might be additional cleanup needs to be done after the tools ran.
///
/// Add these operations here to make sure that they will run before felt
/// exit.
final List<AsyncCallback> cleanupCallbacks = <AsyncCallback>[];

/// Cleanup the remaning processes, close open browsers, delete temp files.
Future<void> cleanup() async {
  // Cleanup remaining processes if any.
  if (processesToCleanUp.isNotEmpty) {
    for (io.Process process in processesToCleanUp) {
      process.kill();
    }
  }
  // Delete temporary directories.
  if (temporaryDirectories.isNotEmpty) {
    for (io.Directory directory in temporaryDirectories) {
      directory.deleteSync(recursive: true);
    }
  }

  for (AsyncCallback callback in cleanupCallbacks) {
    callback.call();
  }
}

/// Resolves paths relative to other paths.
class FilePath {
  /// Resolves relative to the current working directory.
  FilePath.fromCwd(String relativePath)
      : _absolutePath = pathlib.absolute(relativePath);

  /// Resolves relative to the `web_ui` directory.
  FilePath.fromWebUi(String relativePath)
      : _absolutePath = pathlib.join(environment.webUiRootDir.path, relativePath);

  final String _absolutePath;

  /// Absolute version of this path.
  String get absolute => _absolutePath;

  /// This path relative to current working directory.
  String get relativeToCwd => pathlib.relative(_absolutePath);

  /// This path relative to the `web_ui` directory.
  String get relativeToWebUi =>
      pathlib.relative(_absolutePath, from: environment.webUiRootDir.path);

  @override
  int get hashCode => _absolutePath.hashCode;

  @override
  bool operator ==(Object other) {
    return other is FilePath && _absolutePath == other._absolutePath;
  }

  @override
  String toString() => _absolutePath;
}
