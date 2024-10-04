// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';

import 'package:gn_test_wrapper/gn_test_wrapper.dart';
import 'package:meta/meta.dart';

void main(List<String> args) async {
  await runWrapper(args, const DartTest());
}

/// A wrapper for running tests emitted by `dart test --reporter=json`.
///
/// See <https://github.com/dart-lang/test/blob/master/pkgs/test/doc/json_reporter.md>.
@visibleForTesting
final class DartTest extends GnTestWrapper {
  const DartTest({
    super.processManager,
  });

  @override
  List<TestEvent> parseOutputLine(String line) {
    final Object? json;
    try {
      json = jsonDecode(line);
    } on FormatException catch (e) {
      throw FormatException('Failed to parse JSON: $e', line);
    }
    if (json is! Map<String, Object?>) {
      throw FormatException('Expected a JSON object', line);
    }
    // TODO(matanlurey): Parse the object and return events.
    // By default by leaving this blank, only exit codes are interpreted.
    return const [];
  }
}
