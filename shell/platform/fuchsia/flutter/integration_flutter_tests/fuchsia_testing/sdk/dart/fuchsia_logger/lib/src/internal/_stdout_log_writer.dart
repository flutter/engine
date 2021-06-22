// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
//ignore_for_file: import_of_legacy_library_into_null_safe, unnecessary_null_comparison

import 'package:logging/logging.dart';

import '_log_message.dart';
import '_log_writer.dart';

/// A concrete implementation of [LogWriter] which prints the logs to stdout.
class StdoutLogWriter extends LogWriter {
  /// Constructor
  StdoutLogWriter({required Stream<LogRecord> logStream})
      : super(
          logStream: logStream,
          shouldBufferLogs: false,
        );

  @override
  void onMessage(LogMessage message) {
    final scopes = [
      getLevelString(message.record.level),
    ];

    void addToScopes(String? scope) {
      if (scope != null && scope.isNotEmpty) {
        scopes.add(scope);
      }
    }

    addToScopes(message.loggerBaseName);
    addToScopes(message.record.loggerName);
    addToScopes(message.codeLocation);

    message.tags!.forEach(scopes.add);
    String scopesString = scopes.join(':');

    if (message.record.error != null) {
      print(
          '[$scopesString] ${message.record.message}: ${message.record.error}');
    } else {
      print('[$scopesString] ${message.record.message}');
    }

    if (message.record.stackTrace != null) {
      print('${message.record.stackTrace}');
    }
  }
}
