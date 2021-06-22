// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
// ignore_for_file: implementation_imports, import_of_legacy_library_into_null_safe

import 'dart:async';

import 'package:fuchsia_logger/src/internal/_log_message.dart';
import 'package:fuchsia_logger/src/internal/_log_writer.dart';
import 'package:logging/logging.dart';
import 'package:test/test.dart';

//TODO: need a test for setupLogger adding the name as a global tag

void main() {
  group('fuchsia log writer tests', () {
    late Logger logger;

    setUp(() {
      logger = Logger.root;
    });

    tearDown(() {
      logger.clearListeners();
    });

    test('unbuffered logs should write immediately', () {
      bool gotMessage = false;
      StubLogWriter(
          logger: logger,
          onMessageFunc: (_) {
            gotMessage = true;
          });

      logger.info('foo');
      expect(gotMessage, isTrue);
    });

    test('buffered logs should not write immediately', () {
      bool gotMessage = false;
      StubLogWriter(
          logger: logger,
          shouldBufferLogs: true,
          onMessageFunc: (_) {
            gotMessage = true;
          });

      logger.info('foo');
      expect(gotMessage, isFalse);
    });

    test('buffered logs should write after startListening called', () async {
      bool gotMessage = false;
      final completer = Completer();
      final writer = StubLogWriter(
          logger: logger,
          shouldBufferLogs: true,
          onMessageFunc: (_) {
            gotMessage = true;
            completer.complete();
          });

      logger.info('foo');
      expect(gotMessage, isFalse);

      writer.readyToListen();
      await completer.future;

      expect(gotMessage, isTrue);
    });

    test('verify global tags removes null values', () {
      List<String>? tags;

      StubLogWriter(
          logger: logger,
          onMessageFunc: (m) {
            tags = m.tags;
          }).globalTags = ['foo', null, 'bar'];

      logger.info('foo');
      expect(tags, equals(['foo', 'bar']));
    });

    test('verify global tags removes empty values', () {
      List<String>? tags;

      StubLogWriter(
          logger: logger,
          onMessageFunc: (m) {
            tags = m.tags;
          }).globalTags = ['foo', '', 'bar'];

      logger.info('foo');
      expect(tags, equals(['foo', 'bar']));
    });

    test('setting global tags limits the amount of tags', () {
      List<String>? tags;
      final expectedTags = ['foo', 'bar', 'baz', 'buzz'];

      StubLogWriter(
          logger: logger,
          onMessageFunc: (m) {
            tags = m.tags;
          }).globalTags = ['foo', 'bar', 'baz', 'buzz', 'drop'];

      logger.info('foo');
      expect(tags, equals(expectedTags));
    });

    test('setting global tags truncates long tags', () {
      const maxTagLength = 63; // from _log_writer.dart

      List<String>? tags;
      final longTag = ''.padLeft(maxTagLength + 1, 'x');

      StubLogWriter(
          logger: logger,
          onMessageFunc: (m) {
            tags = m.tags;
          }).globalTags = [longTag];

      logger.info('foo');

      expect(tags!.firstWhere((t) => t.length == maxTagLength), isNotNull);
    });

    test('setting forceShowCodeLocation includes code location in tags', () {
      String? codeLocation;

      StubLogWriter(
          logger: logger,
          onMessageFunc: (m) {
            codeLocation = m.codeLocation;
          }).forceShowCodeLocation = true;

      logger.info('foo');
      expect(codeLocation, matches(r'log_writer_test.dart\(\d+\)'));
    });
  });
}

class StubLogWriter extends LogWriter {
  final void Function(LogMessage)? onMessageFunc;

  StubLogWriter({
    required Logger logger,
    this.onMessageFunc,
    shouldBufferLogs = false,
  }) : super(logStream: logger.onRecord, shouldBufferLogs: shouldBufferLogs);

  @override
  void onMessage(LogMessage message) => onMessageFunc!(message);

  void readyToListen() => startListening(onMessageFunc);
}
