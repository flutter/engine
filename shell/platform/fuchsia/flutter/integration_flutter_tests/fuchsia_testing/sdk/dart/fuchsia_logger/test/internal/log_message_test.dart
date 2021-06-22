// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
// ignore_for_file: implementation_imports, import_of_legacy_library_into_null_safe

import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

import 'package:fidl_fuchsia_logger/fidl_async.dart' as fidl_log;
import 'package:fuchsia_logger/src/internal/_log_message.dart';
import 'package:logging/logging.dart';
import 'package:test/test.dart';
import 'package:zircon/zircon.dart';

// 15 sec in nanoseconds
const int _lookBackTimeGap = 15 * 1000 * 1000 * 1000;
const int _socketBufferLength = 2032;

void main() {
  /// the following tests are taken from the initial logging implementation.
  /// They should remain to ensure backwards compatibility
  group('legacy toBytes tests', () {
    test('convert to bytes info', () {
      final message = _makeMessage(Level.INFO, 'foo',
          error: Exception(''), stackTrace: StackTrace.fromString(''));

      final bytes = message.toBytes();
      final buffer = bytes.buffer.asUint8List();
      _validateFixedBlock(buffer, fidl_log.LogLevelFilter.info.$value, 1, 2);

      expect(buffer[32], equals(4));
      // The name of the logger
      expect(utf8.decode(buffer.sublist(33, 37)), equals('TEST'));
      expect(utf8.decode(buffer.sublist(38, 41)), equals('foo'));
      expect(utf8.decode(buffer.sublist(41, 54)), equals(': Exception: '));
      expect(buffer[56], equals(0));
      // Length should be 33 + 5 (TEST) + 4 (foo) + 14 (: Exception: \n)
      expect(bytes.lengthInBytes, equals(56));
    });

    test('convert to bytes exception', () {
      final message = _makeMessage(Level.SHOUT, 'error',
          error: Exception('cause'), stackTrace: StackTrace.fromString(''));

      final bytes = message.toBytes();
      final buffer = bytes.buffer.asUint8List();
      _validateFixedBlock(buffer, fidl_log.LogLevelFilter.error.$value, 1, 2);

      // The name of the logger
      expect(buffer[32], equals(4));
      expect(utf8.decode(buffer.sublist(33, 37)), equals('TEST'));
      int end = 37;

      // dividing 0 byte
      expect(buffer[end++], equals(0));

      int start = end;
      expect(utf8.decode(buffer.sublist(start)),
          matches('error: Exception: cause'));
      end = start + 23;
      expect(buffer[end++], equals(10)); // newline
      expect(buffer[end++], equals(0));
      expect(bytes.lengthInBytes, equals(end));
    });

    test('convert to bytes with stack trace', () {
      const errorMsg = 'this error message plus the stacktrace need to be '
          'long enough to hit the max block size to validate that truncation of long '
          'messages works properly';

      final currentStackTrace = StackTrace.current;
      final longStackTrace = '$currentStackTrace' * 10;
      final message = _makeMessage(Level.SEVERE, errorMsg,
          error: Exception('because'),
          stackTrace: StackTrace.fromString('_STACK_START_ $longStackTrace'));

      final bytes = message.toBytes();
      final buffer = bytes.buffer.asUint8List();
      _validateFixedBlock(buffer, fidl_log.LogLevelFilter.error.$value, 1, 2);

      // The name of the logger
      expect(buffer[32], equals(4));
      expect(utf8.decode(buffer.sublist(33, 37)), equals('TEST'));
      int end = 37;

      // dividing 0 byte
      expect(buffer[end++], equals(0));

      String msg = utf8.decode(buffer.sublist(end));
      expect(msg, startsWith('$errorMsg: Exception: because\n'));
      expect(msg, matches('_STACK_START_'));
      expect(buffer.length, equals(_socketBufferLength));
      expect(
          utf8.decode(
              buffer.sublist(_socketBufferLength - 4, _socketBufferLength - 1)),
          equals('...'));

      expect(bytes.lengthInBytes, _socketBufferLength);
    });

    test('convert to bytes with tags', () {
      final tags = ['tag1', 'tag2'];
      final message = _makeMessage(Level.FINE, 'bar',
          tags: tags,
          error: Exception(''),
          stackTrace: StackTrace.fromString(''));

      final bytes = message.toBytes();
      final buffer = bytes.buffer.asUint8List();
      _validateFixedBlock(buffer, fidl_log.LogLevelFilter.debug.$value, 1, 2);

      // The name of the logger
      expect(buffer[32], equals(4));
      expect(utf8.decode(buffer.sublist(33, 37)), equals('TEST'));
      int start = 37;

      // verify the first tag
      expect(buffer[start], equals(tags[0].length));
      int end = start + buffer[start] + 1;
      start++;
      expect(utf8.decode(buffer.sublist(start, end)), equals(tags[0]));

      // verify the second tag
      start = end;
      expect(buffer[start], equals(tags[1].length));
      end = start + buffer[start] + 1;
      start++;
      expect(utf8.decode(buffer.sublist(start, end)), equals(tags[1]));

      // dividing 0 byte
      expect(buffer[end++], equals(0));

      start = end;
      expect(utf8.decode(buffer.sublist(start, start + 3)), equals('bar'));
      start = start + 3;
      expect(utf8.decode(buffer.sublist(start, start + 13)),
          equals(': Exception: '));
      end = start + 13;
      expect(buffer[end++], equals(10)); // newline
      expect(buffer[end++], equals(0));
      expect(bytes.lengthInBytes, equals(end));
    });

    test('convert to bytes with tags and named Logger', () {
      final tags = ['tag'];
      final message = _makeMessage(
        Level.FINE,
        'bar',
        tags: tags,
        loggerName: 'test-logger',
        error: Exception(''),
        stackTrace: StackTrace.fromString(''),
      );

      final bytes = message.toBytes();
      final buffer = bytes.buffer.asUint8List();
      _validateFixedBlock(buffer, fidl_log.LogLevelFilter.debug.$value, 1, 2);

      // The name of the logger
      expect(buffer[32], equals(4));
      expect(utf8.decode(buffer.sublist(33, 37)), equals('TEST'));
      int start = 37;

      // verify that the named logger gets added after the tags
      expect(buffer[start], equals('test-logger'.length));
      int end = start + buffer[start] + 1;
      start++;
      expect(utf8.decode(buffer.sublist(start, end)), equals('test-logger'));

      // verify the first tag
      start = end;
      expect(buffer[start], equals(tags[0].length));
      end = start + buffer[start] + 1;
      start++;
      expect(utf8.decode(buffer.sublist(start, end)), equals(tags[0]));

      // dividing 0 byte
      expect(buffer[end++], equals(0));

      start = end;
      expect(utf8.decode(buffer.sublist(start, start + 3)), equals('bar'));
      end = start + 3;

      start = end;
      end = start + 13;
      expect(utf8.decode(buffer.sublist(start, end)), equals(': Exception: '));
      expect(buffer[end++], equals(10));
      expect(buffer[end++], equals(0));
      expect(bytes.lengthInBytes, equals(end));
    });
  });
}

/// Convert from little endian format bytes to an unsiged 32 bit int.
int _bytesToInt32(List<int> bytes) {
  ByteData byteData = ByteData(4);
  for (int i = 0; i < 4; i++) {
    byteData.setInt8(i, bytes[i]);
  }
  return byteData.getInt32(0, Endian.little);
}

/// Convert from little endian format bytes to an unsiged 64 bit int.
int _bytesToUint64(List<int> bytes) {
  ByteData byteData = ByteData(8);
  for (int i = 0; i < 8; i++) {
    byteData.setInt8(i, bytes[i]);
  }
  return byteData.getUint64(0, Endian.little);
}

LogMessage _makeMessage(Level level, String message,
        {required Object error,
        required StackTrace stackTrace,
        int processId = 1,
        int threadId = 2,
        String name = 'TEST',
        List<String>? tags,
        String loggerName = ''}) =>
    LogMessage(
      record: LogRecord(level, message, loggerName, error, stackTrace),
      processId: processId,
      threadId: threadId,
      loggerBaseName: name,
      tags: tags,
    );

void _validateFixedBlock(
    List<int> data, int level, int processId, int threadId) {
  expect(_bytesToUint64(data), equals(processId));
  expect(_bytesToUint64(data.sublist(8, 16)), equals(threadId));

  // Log time should be within the last 30 seconds
  int nowNanos = Platform.isFuchsia
      ? System.clockGetMonotonic()
      : DateTime.now().microsecondsSinceEpoch * 1000;
  int logNanos = _bytesToUint64(data.sublist(16, 24));

  expect(logNanos, lessThanOrEqualTo(nowNanos));
  expect(logNanos + _lookBackTimeGap, greaterThan(nowNanos));

  expect(_bytesToInt32(data.sublist(24, 28)), equals(level));
}
