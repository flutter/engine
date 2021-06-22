// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
//ignore_for_file: import_of_legacy_library_into_null_safe, unnecessary_null_comparison

import 'dart:convert' show utf8;
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';

import 'package:fidl_fuchsia_logger/fidl_async.dart' as fidl_log;
import 'package:logging/logging.dart';
import 'package:zircon/zircon.dart';

const int _maxCombinedTags = 5;
const int _maxTagLength = 63;
const int _socketBufferLength = 2032;
const int _unexpectedLoggingLevel = 100;

final Map<Level, int> _enumToFuchsiaLevelMap = <Level, int>{
  Level.FINEST: fidl_log.LogLevelFilter.trace.$value,
  Level.FINER: fidl_log.LogLevelFilter.trace.$value,
  Level.FINE: fidl_log.LogLevelFilter.debug.$value,
  Level.CONFIG: fidl_log.LogLevelFilter.debug.$value,
  Level.INFO: fidl_log.LogLevelFilter.info.$value,
  Level.WARNING: fidl_log.LogLevelFilter.warn.$value,
  Level.SEVERE: fidl_log.LogLevelFilter.error.$value,
  Level.SHOUT: fidl_log.LogLevelFilter.error.$value,
};

/// A wrapper around [LogRecord] which appends additional data. This
/// is what is sent to the log writer when a record is received.
class LogMessage {
  /// The initial log record
  final LogRecord record;

  /// The base name for the logger. This is the name that is assigned in
  /// setupLogger whereas the name in the record is the name that comes from
  /// the dart logger instance.
  final String? loggerBaseName;

  /// A string that can be included to identify the call site of this log message
  final String? codeLocation;

  /// Any additional tags to append to the record. When the record it sent to the
  /// logger it will include these tags after the name and code location if they are
  /// present. Any tags which are over the limit of the [_maxCombinedTags] will
  /// be dropped.
  final List<String>? tags;

  /// The id of the process which this log message is associated with
  final int processId;

  /// The id of the thread which this log message is associated with
  final int threadId;

  /// The time that this message was created
  final int systemTime = Platform.isFuchsia
      ? System.clockGetMonotonic()
      : DateTime.now().microsecondsSinceEpoch * 1000;

  /// The default constructor
  LogMessage({
    required this.record,
    required this.processId,
    required this.threadId,
    this.loggerBaseName,
    this.codeLocation,
    this.tags = const [],
  });

  /// Converts this to a ByteData which can be used to send the message to the
  /// log socket.
  ByteData toBytes() {
    ByteData bytes = ByteData(_socketBufferLength)
      ..setUint64(0, processId, Endian.little)
      ..setUint64(8, threadId, Endian.little)
      ..setUint64(16, systemTime, Endian.little)
      ..setInt32(24, _convertLogLevel(record.level), Endian.little)
      ..setUint32(28, 0, Endian.little); // TODO(120860552) droppedLogs
    int byteOffset = 32;

    int totalTagCount = 0;
    void addTag(String? tag) {
      if (totalTagCount < _maxCombinedTags) {
        byteOffset = _setTag(bytes, byteOffset, tag);
        totalTagCount++;
      }
    }

    addTag(loggerBaseName);
    addTag(record.loggerName);
    addTag(codeLocation);

    if (tags != null) {
      tags!.forEach(addTag);
    }

    // We need to skip the local tags section since we do not support them
    bytes.setUint8(byteOffset++, 0);

    // Write message
    byteOffset = _setString(bytes, byteOffset, record.message,
        _socketBufferLength - byteOffset - 1);
    if (record.error != null) {
      byteOffset = _setString(
          bytes, byteOffset, ': ', _socketBufferLength - byteOffset - 1);
      byteOffset = _setString(bytes, byteOffset, record.error.toString(),
          _socketBufferLength - byteOffset - 1);
    }
    if (record.stackTrace != null) {
      byteOffset = _setString(
          bytes, byteOffset, '\n', _socketBufferLength - byteOffset - 1);
      byteOffset = _setString(bytes, byteOffset, record.stackTrace.toString(),
          _socketBufferLength - byteOffset - 1);
    }
    bytes.setUint8(byteOffset++, 0);
    return bytes.buffer.asByteData(0, byteOffset);
  }

  int _convertLogLevel(Level logLevel) =>
      _enumToFuchsiaLevelMap[logLevel] ?? _unexpectedLoggingLevel;

  /// Write a string to ByteData with a leading length byte. Return the
  /// byteOffstet to use for the next value. Wrie a non-terminated string to
  /// ByteData. Return the byteOffset to use for the terminating byte or the
  /// next value.
  int _setString(
      ByteData bytes, int firstByteOffset, String value, int maxLen) {
    if (value.isEmpty) {
      return firstByteOffset;
    }
    List<int> charBytes = utf8.encode(value);
    int len = min(charBytes.length, maxLen);
    int byteOffset = firstByteOffset;
    for (int i = 0; i < len; i++) {
      bytes.setUint8(byteOffset++, charBytes[i]);
    }
    // If the string was truncated (and there is space), add an elipsis character.
    if (len < charBytes.length && len >= 3) {
      const int period = 46; // UTF8 value for '.'
      for (int i = 1; i <= 3; i++) {
        bytes.setUint8(byteOffset - i, period);
      }
    }
    return byteOffset;
  }

  int _setTag(ByteData bytes, int byteOffset, String? tag) {
    if (tag == null || tag == 'null' || tag.isEmpty) {
      return byteOffset;
    }
    int nextOffset = _setString(bytes, byteOffset + 1, tag, _maxTagLength);
    bytes.setUint8(byteOffset, nextOffset - byteOffset - 1);
    return nextOffset;
  }
}
