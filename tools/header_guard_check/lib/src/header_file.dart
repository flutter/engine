// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:meta/meta.dart';
import 'package:path/path.dart' as p;
import 'package:source_span/source_span.dart';

/// Represents a C++ header file, i.e. a file on disk that ends in `.h`.
@immutable
final class HeaderFile {
  /// Creates a new header file from the given [path].
  const HeaderFile.from(
    this.path, {
    required this.guard,
    required this.pragmaOnce,
  });

  /// Parses the given [path] as a header file.
  ///
  /// Throws an [ArgumentError] if the file does not exist.
  factory HeaderFile.parse(String path) {
    final io.File file = io.File(path);
    if (!file.existsSync()) {
      throw ArgumentError.value(path, 'path', 'File does not exist.');
    }

    final String contents = file.readAsStringSync();
    final SourceFile sourceFile = SourceFile.fromString(contents, url: p.toUri(path));
    return HeaderFile.from(
      path,
      guard: _parseGuard(sourceFile),
      pragmaOnce: _parsePragmaOnce(sourceFile),
    );
  }

  /// Parses the header guard of the given [sourceFile].
  static HeaderGuardSpans? _parseGuard(SourceFile sourceFile) {
    SourceSpan? ifndefSpan;
    SourceSpan? defineSpan;
    SourceSpan? endifSpan;

    // Iterate over the lines in the file.
    for (int i = 0; i < sourceFile.lines; i++) {
      final int start = sourceFile.getOffset(i);
      final int end = i == sourceFile.lines - 1
          ? sourceFile.length
          : sourceFile.getOffset(i + 1) - 1;
      final String line = sourceFile.getText(start, end);

      // Check if the line is a header guard directive.
      if (line.startsWith('#ifndef')) {
        ifndefSpan = sourceFile.span(start, end);
      } else if (line.startsWith('#define')) {
        defineSpan = sourceFile.span(start, end);
        break;
      }
    }

    // If we found no header guard, return null.
    if (ifndefSpan == null) {
      return null;
    }

    // Now iterate backwards to find the (last) #endif directive.
    for (int i = sourceFile.lines - 1; i > 0; i--) {
      final int start = sourceFile.getOffset(i);
      final int end = i == sourceFile.lines - 1 ?
          sourceFile.length :
          sourceFile.getOffset(i + 1) - 1;
      final String line = sourceFile.getText(start, end);

      // Check if the line is a header guard directive.
      if (line.startsWith('#endif')) {
        endifSpan = sourceFile.span(start, end);
        break;
      }
    }

    return HeaderGuardSpans(
      ifndefSpan: ifndefSpan,
      defineSpan: defineSpan,
      endifSpan: endifSpan,
    );
  }

  /// Parses the `#pragma once` directive of the given [sourceFile].
  static SourceSpan? _parsePragmaOnce(SourceFile sourceFile) {
    // Iterate over the lines in the file.
    for (int i = 0; i < sourceFile.lines; i++) {
      final int start = sourceFile.getOffset(i);
      final int end = i == sourceFile.lines - 1
          ? sourceFile.length
          : sourceFile.getOffset(i + 1) - 1;
      final String line = sourceFile.getText(start, end);

      // Check if the line is a header guard directive.
      if (line.startsWith('#pragma once')) {
        return sourceFile.span(start, end);
      }
    }

    return null;
  }

  /// Path to the file on disk.
  final String path;

  /// The header guard span, if any.
  ///
  /// This is `null` if the file does not have a header guard.
  final HeaderGuardSpans? guard;

  /// The `#pragma once` directive, if any.
  ///
  /// This is `null` if the file does not have a `#pragma once` directive.
  final SourceSpan? pragmaOnce;

  @override
  bool operator ==(Object other) {
    return other is HeaderFile &&
        path == other.path &&
        guard == other.guard &&
        pragmaOnce == other.pragmaOnce;
  }

  @override
  int get hashCode => Object.hash(path, guard, pragmaOnce);

  @override
  String toString() {
    return 'HeaderFile(\n'
        '  path:       $path\n'
        '  guard:      $guard\n'
        '  pragmaOnce: $pragmaOnce\n'
        ')';
  }
}

/// Source elements that are part of a header guard.
@immutable
final class HeaderGuardSpans {
  /// Collects the source spans of the header guard directives.
  const HeaderGuardSpans({
    required this.ifndefSpan,
    required this.defineSpan,
    required this.endifSpan,
  });

  /// Location of the `#ifndef` directive.
  final SourceSpan? ifndefSpan;

  /// Location of the `#define` directive.
  final SourceSpan? defineSpan;

  /// Location of the `#endif` directive.
  final SourceSpan? endifSpan;

  @override
  bool operator ==(Object other) {
    return other is HeaderGuardSpans &&
        ifndefSpan == other.ifndefSpan &&
        defineSpan == other.defineSpan &&
        endifSpan == other.endifSpan;
  }

  @override
  int get hashCode => Object.hash(ifndefSpan, defineSpan, endifSpan);

  @override
  String toString() {
    return 'HeaderGuardSpans(\n'
        '  #ifndef: $ifndefSpan\n'
        '  #define: $defineSpan\n'
        '  #endif:  $endifSpan\n'
        ')';
  }

  /// Returns the value of the `#ifndef` directive.
  ///
  /// For example, `#ifndef FOO_H_`, this will return `FOO_H_`.
  ///
  /// If the span is not a valid `#ifndef` directive, `null` is returned.
  String? get ifndefValue {
    final String? value = ifndefSpan?.text;
    if (value == null) {
      return null;
    }
    if (!value.startsWith('#ifndef ')) {
      return null;
    }
    return value.substring('#ifndef '.length);
  }

  /// Returns the value of the `#define` directive.
  ///
  /// For example, `#define FOO_H_`, this will return `FOO_H_`.
  ///
  /// If the span is not a valid `#define` directive, `null` is returned.
  String? get defineValue {
    final String? value = defineSpan?.text;
    if (value == null) {
      return null;
    }
    if (!value.startsWith('#define ')) {
      return null;
    }
    return value.substring('#define '.length);
  }

  /// Returns the value of the `#endif` directive.
  ///
  /// For example, `#endif  // FOO_H_`, this will return `FOO_H_`.
  ///
  /// If the span is not a valid `#endif` directive, `null` is returned.
  String? get endifValue {
    final String? value = endifSpan?.text;
    if (value == null) {
      return null;
    }
    if (!value.startsWith('#endif  // ')) {
      return null;
    }
    return value.substring('#endif  // '.length);
  }
}
