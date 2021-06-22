// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:fuchsia_logger/logger.dart';
import 'package:fuchsia_services/services.dart';
import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_logger/fidl_async.dart' as fidl_log;
import 'package:test/test.dart';

const String testTag = 'log_integ';

void main() {
  group('log messages', () {
    test('messages reach syslog', () async {
      setupLogger(name: testTag);
      log
        ..warning('warning from integ test')
        ..info('info from integ test');

      final foundLogs = await logStream().take(2).toList();
      // sort logs by time to account for out of order arrival
      foundLogs.sort((a, b) => a.time.compareTo(b.time));

      final firstLog = foundLogs.elementAt(0);
      expect(firstLog.msg, equals('warning from integ test'));
      // some build targets include additional debug tags such as code location
      expect(firstLog.tags, contains(testTag));
      expect(firstLog.severity, equals(fidl_log.LogLevelFilter.warn.$value));

      final secondLog = foundLogs.elementAt(1);
      expect(secondLog.msg, equals('info from integ test'));
      expect(secondLog.tags, contains(testTag));
      expect(secondLog.severity, equals(fidl_log.LogLevelFilter.info.$value));
    });
  });
}

/// Retrieves a stream of logs from syslog which have the [testTag] tag.
Stream<fidl_log.LogMessage> logStream() {
  final logProxy = fidl_log.LogProxy();
  Incoming.fromSvcPath()
    ..connectToService(logProxy)
    ..close();

  final interfacePair = InterfacePair<fidl_log.LogListenerSafe>();
  logProxy.listenSafe(
      interfacePair.handle,
      fidl_log.LogFilterOptions(
        filterByPid: false,
        pid: 0,
        filterByTid: false,
        tid: 0,
        verbosity: 0,
        minSeverity: fidl_log.LogLevelFilter.info,
        tags: [testTag],
      ));
  logProxy.ctrl.close();
  return LogAsserter(interfacePair.request).logs;
}

/// A [LogListenerSafe] implementation that places received logs into
/// a stream.
class LogAsserter extends fidl_log.LogListenerSafe {
  final _binding = fidl_log.LogListenerSafeBinding();
  final StreamController<fidl_log.LogMessage> _logStreamController =
      StreamController();

  LogAsserter(InterfaceRequest<fidl_log.LogListenerSafe> request) {
    _binding.bind(this, request);
  }

  @override
  Future<void> log(fidl_log.LogMessage log) async => _handleLog(log);

  @override
  Future<void> logMany(List<fidl_log.LogMessage> logs) async =>
      logs.forEach(_handleLog);

  @override
  Future<void> done() => _logStreamController.close();

  void _handleLog(fidl_log.LogMessage log) {
    _logStreamController.add(log);
  }

  Stream<fidl_log.LogMessage> get logs => _logStreamController.stream;
}
