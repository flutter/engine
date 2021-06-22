// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
// ignore_for_file: implementation_imports, import_of_legacy_library_into_null_safe

import 'dart:async';

import 'package:fuchsia_logger/src/internal/_stdout_log_writer.dart';
import 'package:logging/logging.dart';
import 'package:test/test.dart';

void main() {
  group('log messages', () {
    test(
        'finest message',
        () => _overridePrint((logger, _) => logger.finest('foo'),
            (l) => expect(l, '[FINEST] foo')));

    test(
        'finer message',
        () => _overridePrint((logger, _) => logger.finer('foo'),
            (l) => expect(l, '[FINER] foo')));

    test(
        'fine message',
        () => _overridePrint(
            (logger, _) => logger.fine('foo'), (l) => expect(l, '[FINE] foo')));

    test(
        'info message',
        () => _overridePrint(
            (logger, _) => logger.info('foo'), (l) => expect(l, '[INFO] foo')));

    test(
        'config message',
        () => _overridePrint((logger, _) => logger.config('foo'),
            (l) => expect(l, '[CONFIG] foo')));

    test(
        'warning message',
        () => _overridePrint((logger, _) => logger.warning('foo'),
            (l) => expect(l, '[WARNING] foo')));

    test(
        'severe message',
        () => _overridePrint((logger, _) => logger.severe('foo'),
            (l) => expect(l, '[ERROR] foo')));

    test(
        'shout message',
        () => _overridePrint((logger, _) => logger.shout('foo'),
            (l) => expect(l, '[FATAL] foo')));

    test(
        'includes error object',
        () => _overridePrint(
            (logger, _) => logger.severe('foo', 'fail message'),
            (l) => expect(l, '[ERROR] foo: fail message')));

    test('includes stack trace', () {
      final lines = <String>[];
      _overridePrint(
          (logger, _) => logger.severe(
              'foo', 'fail message', StackTrace.fromString('my stacktrace')),
          lines.add);
      expect(lines.length, 2);
      expect(lines[1], 'my stacktrace');
    });

    test('includes code location', () {
      _overridePrint((logger, writer) {
        writer.forceShowCodeLocation = true;
        logger.info('foo');
      }, (l) => expect(l, matches(r'INFO:stdout_log_writer_test.dart\(\d+\)')));
    });

    test('includes named logger name', () {
      _overridePrint((logger, writer) {
        logger.info('foo');
      }, (l) => expect(l, '[INFO:named-logger] foo'),
          namedLogger: Logger('named-logger'));
    });
  });

  group('tags', () {
    test('setting global tags adds them to the log line', () {
      _overridePrint((logger, writer) {
        writer.globalTags = ['baz', 'bar'];
        logger.info('foo');
      }, (l) => expect(l, '[INFO:baz:bar] foo'));
    });
  });
}

// helper method for capturing stdout. Code executed within the [zoned] function
// will pass the logged output to the [receiver] function with the line that
// would have gone to stdout.
void _overridePrint(
  void Function(Logger, StdoutLogWriter) zoned,
  void Function(String) receiver, {
  Logger? namedLogger,
}) {
  runZoned(
    () {
      final logger = namedLogger ?? Logger.root;
      zoned(logger, StdoutLogWriter(logStream: logger.onRecord));
      logger.clearListeners();
    },
    zoneSpecification: ZoneSpecification(
      print: (Zone self, ZoneDelegate parent, Zone zone, String line) {
        receiver(line);
      },
    ),
  );
}
