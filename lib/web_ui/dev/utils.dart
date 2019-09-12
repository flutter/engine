// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io' as io;

import 'package:path/path.dart' as path;

import 'environment.dart';

class FilePath {
  FilePath.fromCwd(String filePath) : _path = path.absolute(filePath);
  FilePath.fromWebUi(String filePath)
      : _path = path.join(environment.webUiRootDir.path, filePath);

  final String _path;

  String get relativeToCwd => path.relative(_path);
  String get relativeToWebUi =>
      path.relative(_path, from: environment.webUiRootDir.path);

  String get absolute => _path;

  @override
  bool operator ==(dynamic other) {
    if (other is String) {
      return _path == other;
    }
    if (other is FilePath) {
      return _path == other._path;
    }
    return false;
  }

  @override
  String toString() => _path;
}

Future<int> runProcess(
  String executable,
  List<String> arguments, {
  String workingDirectory,
}) async {
  final io.Process process = await io.Process.start(
    executable,
    arguments,
    workingDirectory: workingDirectory,
  );
  return _forwardIOAndWait(process);
}

Future<int> _forwardIOAndWait(io.Process process) {
  final StreamSubscription stdoutSub = process.stdout.listen(io.stdout.add);
  final StreamSubscription stderrSub = process.stderr.listen(io.stderr.add);
  return process.exitCode.then<int>((int exitCode) {
    stdoutSub.cancel();
    stderrSub.cancel();
    return exitCode;
  });
}
