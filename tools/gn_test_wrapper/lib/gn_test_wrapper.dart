// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';

import 'package:async/async.dart';
import 'package:collection/collection.dart';
import 'package:meta/meta.dart';
import 'package:process/process.dart';

import 'src/run_wrapper_internal.dart';

/// Entrypoint from the command line to run a test executable produced by GN.
///
/// ## Example
///
/// ```dart
/// void main(List<String> args) async => runWrapper(args, MyGnTestWrapper());
/// ```
Future<void> runWrapper(
  List<String> args,
  GnTestWrapper wrapper, {
  StringSink? stdout,
  StringSink? stderr,
}) async {
  await runWrapperInternal(args, wrapper, stdout: stdout, stderr: stderr);
}

/// Wraps and standardizes the invocation of a test executable produced by GN.
///
/// A [GnTestWrapper] is a stateless immutable object that represents an
/// intermediate interface between a test runner (such as `dart test` or a C++
/// `gtest` executable) and a runner agnostic tester (such as `et test`).
///
/// The contract of a [GnTestWrapper] is:
///
/// - It starts a process based on running a test executable at a given path.
/// - The process must not require any command line arguments or input from the
///   user (i.e. stdin). If it requires environment variables, they should be
///   set by the parent process before invoking the test executable.
/// - The process must emit UTF-8 encoded text to stdout, separated by newlines.
/// - Each line of output, if significant, may be parsed into a [TestEvent].
/// - The process must exit with a status code of 0 if all tests passed, or a
///   non-zero status code if any tests failed.
///
/// ## Example
///
/// ```dart
/// final class MyGnTestWrapper extends GnTestWrapper {
///   MyGnTestWrapper({
///     super.processManager,
///   });
///
///   @override
///   List<TestEvent> parseOutputLine(String line) {
///     // Parse the output line and return a list of TestEvent objects.
///   }
/// }
/// ```
@immutable
abstract base class GnTestWrapper {
  /// Creates a new [GnTestWrapper].
  const GnTestWrapper({
    ProcessManager? processManager,
  }) : _processManager = processManager ?? const LocalProcessManager();

  /// Runs and streams the output of the test executable at the given [path].
  ///
  /// The returned stream will emit [TestEvent]s as the test executable runs.
  @nonVirtual
  Stream<TestEvent> runTest(String path) {
    return StreamCompleter.fromFuture(_runTest(path));
  }

  /// Avoids having to either use `async*` or nested manual `Future`s.
  Future<Stream<TestEvent>> _runTest(String path) async {
    final controller = StreamController<TestEvent>();
    final process = await _processManager.start([path]);
    process.stdout
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((line) {
      try {
        parseOutputLine(line).forEach(controller.add);
      } catch (error, stack) {
        controller.addError(error, stack);
      }
    }, onDone: () {
      controller.add(const TestCompleteEvent(TestStatus.passed));
      controller.close();
    }, onError: (Object error, StackTrace stack) {
      controller.addError(error, stack);
    });
    return controller.stream;
  }

  final ProcessManager _processManager;

  /// Parses a line of output from the test executable as test events.
  ///
  /// To implement this method, you should parse the given [line] and return a
  /// list of [TestEvent] objects that represent the events that occurred; for
  /// example, you could parse the individual line as JSON, and then determine
  /// what kind of event occurred based on the JSON object's structure.
  @protected
  List<TestEvent> parseOutputLine(String line);
}

/// An event produced by [GnTestWrapper.runTest].
@immutable
sealed class TestEvent {
  /// Creates a new [TestEvent].
  factory TestEvent.fromJson(Map<String, Object?> json) {
    switch (json['type'] as String?) {
      case 'complete':
        return TestCompleteEvent.fromJson(json);
      default:
        throw FormatException('Invalid "type" field', json);
    }
  }

  /// Converts this event to a JSON-serializable object.
  Map<String, Object?> toJson();
}

/// Status of a test.
enum TestStatus {
  /// The test passed.
  passed,

  /// The test failed.
  failed;
}

/// An event indicating that a test has completed
final class TestCompleteEvent implements TestEvent {
  /// Creates a new [TestCompleteEvent].
  const TestCompleteEvent(this.status);

  /// Creates a new [TestCompleteEvent] from a JSON-serializable object.
  factory TestCompleteEvent.fromJson(Map<String, Object?> json) {
    final status = json['status'] as String?;
    if (status == null) {
      throw FormatException('Missing "status" field', json);
    }
    final statusValue = TestStatus.values.firstWhereOrNull(
      (value) => value.name == status,
    );
    if (statusValue == null) {
      throw FormatException('Invalid "status" field', json);
    }
    return TestCompleteEvent(statusValue);
  }

  /// The status of the test.
  final TestStatus status;

  @override
  bool operator ==(Object other) {
    return other is TestCompleteEvent && other.status == status;
  }

  @override
  int get hashCode => status.hashCode;

  @override
  String toString() => 'TestCompleteEvent($status)';

  @override
  Map<String, Object?> toJson() {
    return {
      'type': 'complete',
      'status': status.name,
    };
  }
}
