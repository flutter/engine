// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:meta/meta.dart';

import '../gn_test_wrapper.dart';

@internal
Future<void> runWrapperInternal(
  List<String> args,
  GnTestWrapper wrapper, {
  StringSink? stdout,
  StringSink? stderr,
}) async {
  stdout ??= io.stdout;
  stderr ??= io.stderr;

  final String path;
  switch (args) {
    case [final value]:
      path = value;
    default:
      stderr.writeln('Usage: dart run <wrapper> <path>');
      io.exitCode = 1;
      return;
  }

  // Stream events from the test executable.
  stderr.writeln('[${wrapper.runtimeType}] Running test: $path');
  try {
    await for (final event in wrapper.runTest(path)) {
      switch (event) {
        case TestCompleteEvent(:final status):
          switch (status) {
            case TestStatus.passed:
              io.exitCode = 0;
              stderr.writeln('PASS');
            case TestStatus.failed:
              io.exitCode = 1;
              stderr.writeln('FAIL');
          }
      }
    }
  } on FormatException catch (e) {
    stderr.writeln(e.message);
    io.exitCode = 1;
  }
}
