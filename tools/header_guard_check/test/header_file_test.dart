// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:header_guard_check/src/header_file.dart';
import 'package:litetest/litetest.dart';
import 'package:path/path.dart' as p;
import 'package:source_span/source_span.dart';

Future<int> main(List<String> args) async {
  void withTestFile(String path, String contents, void Function(io.File) fn) {
    // Create a temporary file and delete it when we're done.
    final io.Directory tempDir = io.Directory.systemTemp.createTempSync('header_guard_check_test');
    final io.File file = io.File(p.join(tempDir.path, path));
    file.writeAsStringSync(contents);
    try {
      fn(file);
    } finally {
      tempDir.deleteSync(recursive: true);
    }
  }

  group('HeaderGuardSpans', () {
    test('parses #ifndef', () {
      const String input = '#ifndef FOO_H_';
      final HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: SourceSpanWithContext(
          SourceLocation(0),
          SourceLocation(input.length),
          input,
          input,
        ),
        defineSpan: null,
        endifSpan: null,
      );
      expect(guard.ifndefValue, 'FOO_H_');
    });

    test('ignores #ifndef if omitted', () {
      const HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: null,
        endifSpan: null,
      );
      expect(guard.ifndefValue, isNull);
    });

    test('ignores #ifndef if invalid', () {
      const String input = '#oops FOO_H_';
      final HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: SourceSpanWithContext(
          SourceLocation(0),
          SourceLocation(input.length),
          input,
          input,
        ),
        defineSpan: null,
        endifSpan: null,
      );
      expect(guard.ifndefValue, isNull);
    });

    test('parses #define', () {
      const String input = '#define FOO_H_';
      final HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: SourceSpanWithContext(
          SourceLocation(0),
          SourceLocation(input.length),
          input,
          input,
        ),
        endifSpan: null,
      );
      expect(guard.defineValue, 'FOO_H_');
    });

    test('ignores #define if omitted', () {
      const HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: null,
        endifSpan: null,
      );
      expect(guard.defineValue, isNull);
    });

    test('ignores #define if invalid', () {
      const String input = '#oops FOO_H_';
      final HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: SourceSpanWithContext(
          SourceLocation(0),
          SourceLocation(input.length),
          input,
          input,
        ),
        endifSpan: null,
      );
      expect(guard.defineValue, isNull);
    });

    test('parses #endif', () {
      const String input = '#endif  // FOO_H_';
      final HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: null,
        endifSpan: SourceSpanWithContext(
          SourceLocation(0),
          SourceLocation(input.length),
          input,
          input,
        ),
      );
      expect(guard.endifValue, 'FOO_H_');
    });

    test('ignores #endif if omitted', () {
      const HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: null,
        endifSpan: null,
      );
      expect(guard.endifValue, isNull);
    });

    test('ignores #endif if invalid', () {
      const String input = '#oops  // FOO_H_';
      final HeaderGuardSpans guard = HeaderGuardSpans(
        ifndefSpan: null,
        defineSpan: null,
        endifSpan: SourceSpanWithContext(
          SourceLocation(0),
          SourceLocation(input.length),
          input,
          input,
        ),
      );
      expect(guard.endifValue, isNull);
    });
  });

  group('HeaderFile', () {
    test('parses a header file with a valid guard', () {
      final String input = <String>[
        '#ifndef FOO_H_',
        '#define FOO_H_',
        '',
        '#endif  // FOO_H_',
      ].join('\n');
      withTestFile('foo.h', input, (io.File file) {
        final HeaderFile headerFile = HeaderFile.parse(file.path);
        expect(headerFile.guard!.ifndefValue, 'FOO_H_');
        expect(headerFile.guard!.defineValue, 'FOO_H_');
        expect(headerFile.guard!.endifValue, 'FOO_H_');
      });
    });

    test('parses a header file with an invalid #endif', () {
      final String input = <String>[
        '#ifndef FOO_H_',
        '#define FOO_H_',
        '',
        // No comment after the #endif.
        '#endif',
      ].join('\n');
      withTestFile('foo.h', input, (io.File file) {
        final HeaderFile headerFile = HeaderFile.parse(file.path);
        expect(headerFile.guard!.ifndefValue, 'FOO_H_');
        expect(headerFile.guard!.defineValue, 'FOO_H_');
        expect(headerFile.guard!.endifValue, isNull);
      });
    });

    test('parses a header file with a missing #define', () {
      final String input = <String>[
        '#ifndef FOO_H_',
        // No #define.
        '',
        '#endif  // FOO_H_',
      ].join('\n');
      withTestFile('foo.h', input, (io.File file) {
        final HeaderFile headerFile = HeaderFile.parse(file.path);
        expect(headerFile.guard!.ifndefValue, 'FOO_H_');
        expect(headerFile.guard!.defineValue, isNull);
        expect(headerFile.guard!.endifValue, 'FOO_H_');
      });
    });

    test('parses a header file with a missing #ifndef', () {
      final String input = <String>[
        // No #ifndef.
        '#define FOO_H_',
        '',
        '#endif  // FOO_H_',
      ].join('\n');
      withTestFile('foo.h', input, (io.File file) {
        final HeaderFile headerFile = HeaderFile.parse(file.path);
        expect(headerFile.guard, isNull);
      });
    });

    test('parses a header file with a #pragma once', () {
      final String input = <String>[
        '#pragma once',
        '',
      ].join('\n');
      withTestFile('foo.h', input, (io.File file) {
        final HeaderFile headerFile = HeaderFile.parse(file.path);
        expect(headerFile.pragmaOnce, isNotNull);
      });
    });
  });

  return 0;
}
