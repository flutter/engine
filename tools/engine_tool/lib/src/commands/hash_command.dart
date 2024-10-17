// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import '../../engine_hash.dart';
import 'command.dart';

/// Returns the hash signature of the engine sources used for downloading assets.
final class HashCommand extends CommandBase {
  /// Constructs the 'hash' command.
  HashCommand({
    required super.environment,
    super.usageLineLength,
  });

  @override
  String get name => 'hash';

  @override
  String get description => 'Generates the engine hash signature.';

  @override
  Future<int> run() async {
    final result = await engineHash(
      (List<String> command) async =>
          environment.processRunner.processManager.run(
        command,
        workingDirectory: environment.engine.flutterDir.path,
        stdoutEncoding: utf8,
      ),
    );

    if (result.error != null) {
      environment.logger.fatal(result.error);
    }

    environment.stdout.write(result.result);
    return 0;
  }
}
