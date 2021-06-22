// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
// ignore_for_file: import_of_legacy_library_into_null_safe

import 'dart:async';

import 'package:logging/logging.dart';
import 'package:fidl_fuchsia_logger/fidl_async.dart' as fidl_logger;
import 'package:fuchsia_services/services.dart';
import 'package:zircon/zircon.dart' as zircon;

import '_log_message.dart';
import '_log_writer.dart';

/// A concrete implementation of [LogWriter] which sends logs to
/// the fuchsia system logger. This log writer will buffer logs until
/// a connection has been established at which time it will send all
/// the buffered logs.
class FuchsiaLogWriter extends LogWriter {
  zircon.Socket? _socket;

  /// Constructor
  FuchsiaLogWriter({required Stream<LogRecord> logStream})
      : super(
          logStream: logStream,
          shouldBufferLogs: true,
        ) {
    _connectToSysLogger();
  }

  void _connectToSysLogger() {
    final proxy = fidl_logger.LogSinkProxy();
    Incoming.fromSvcPath()
      ..connectToService(proxy)
      ..close();

    final socketPair = zircon.SocketPair(zircon.Socket.DATAGRAM);
    proxy.connect(socketPair.second!).then((_) {
      _socket = socketPair.first;
      startListening(onMessage);
    }).catchError((e) {
      print('[WARN] Unable to get socket from system logger');
      throw e;
    });
  }

  @override
  void onMessage(LogMessage message) => _socket!.write(message.toBytes());
}
