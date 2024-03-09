// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;
import 'dart:typed_data';

import 'package:crypto/crypto.dart' as crypto;
import 'package:path/path.dart' as p;
import 'package:process_runner/process_runner.dart';

import 'environment.dart';
import 'logger.dart';

/// Check whether the DEPS file has been changed since the last gclient sync.
bool dependenciesUpdated(Environment environment) {
  try {
    // The DEPS.sha256 file contains a SHA-256 hash of the DEPS file.
    // It is created by gclient runhooks.
    final String hashPath = p.join(
      environment.engine.flutterDir.path, 'build', 'DEPS.sha256',
    );
    final io.File hashFile = io.File(hashPath);
    final String previousHash = hashFile.readAsStringSync().toLowerCase();

    // Find the DEPS file's latest hash.
    final String depsPath = p.join(
      environment.engine.flutterDir.path, 'DEPS',
    );
    final Uint8List depsBytes = io.File(depsPath).readAsBytesSync();
    final crypto.Digest latestDigest = crypto.sha256.convert(depsBytes);
    final String latestHash = latestDigest.toString().toLowerCase();

    return latestHash == previousHash;
  } catch (_) {
    return false;
  }
}

/// Update Flutter engine dependencies. Returns an exit code.
Future<int> fetchDependencies(
  Environment environment, {
  bool verbose = false,
}) async {
  if (!environment.processRunner.processManager.canRun('gclient')) {
    environment.logger.error('Cannot find the gclient command in your path');
    return 1;
  }

  environment.logger.status('Fetching dependencies... ', newline: verbose);

  Spinner? spinner;
  ProcessRunnerResult result;
  try {
    if (!verbose) {
      spinner = environment.logger.startSpinner();
    }

    result = await environment.processRunner.runProcess(
      <String>[
        'gclient',
        'sync',
        '-D',
      ],
      runInShell: true,
      startMode: verbose
        ? io.ProcessStartMode.inheritStdio
        : io.ProcessStartMode.normal,
    );
  } finally {
    spinner?.finish();
  }

  if (result.exitCode != 0) {
    environment.logger.error('Fetching dependencies failed.');

    // Verbose mode already logged output by making the child process inherit
    // this process's stdio handles.
    if (!verbose) {
      environment.logger.error('Output:\n${result.output}');
    }
  }

  return result.exitCode;
}
