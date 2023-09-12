// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io' as io;

import 'package:process/process.dart';

/// A fake implementation of [ProcessManager] that allows control for testing.
final class FakeProcessManager implements ProcessManager {
  FakeProcessManager({
    io.ProcessResult Function(List<String> command) onRun = unhandledRun,
    io.Process Function(List<String> command) onStart = unhandledStart,
  }) : _onRun = onRun, _onStart = onStart;

  static io.ProcessResult unhandledRun(List<String> command) {
    throw UnsupportedError('Unhandled run: ${command.join(' ')}');
  }

  static io.Process unhandledStart(List<String> command) {
    throw UnsupportedError('Unhandled start: ${command.join(' ')}');
  }

  final io.ProcessResult Function(List<String> command) _onRun;
  final io.Process Function(List<String> command) _onStart;

  @override
  bool canRun(Object? executable, {String? workingDirectory}) => true;

  @override
  bool killPid(int pid, [io.ProcessSignal signal = io.ProcessSignal.sigterm]) => true;

  @override
  Future<io.ProcessResult> run(
    List<Object> command, {
    String? workingDirectory,
    Map<String, String>? environment,
    bool includeParentEnvironment = true,
    bool runInShell = false,
    Encoding stdoutEncoding = io.systemEncoding,
    Encoding stderrEncoding = io.systemEncoding,
  }) async {
    return runSync(
      command,
      workingDirectory: workingDirectory,
      environment: environment,
      includeParentEnvironment: includeParentEnvironment,
      runInShell: runInShell,
      stdoutEncoding: stdoutEncoding,
      stderrEncoding: stderrEncoding,
    );
  }

  @override
  io.ProcessResult runSync(
    List<Object> command, {
    String? workingDirectory,
    Map<String, String>? environment,
    bool includeParentEnvironment = true,
    bool runInShell = false,
    Encoding stdoutEncoding = io.systemEncoding,
    Encoding stderrEncoding = io.systemEncoding,
  }) {
    return _onRun(command.map((Object o) => '$o').toList());
  }

  @override
  Future<io.Process> start(
    List<Object> command, {
    String? workingDirectory,
    Map<String, String>? environment,
    bool includeParentEnvironment = true,
    bool runInShell = false,
    io.ProcessStartMode mode = io.ProcessStartMode.normal,
  }) async {
    return _onStart(command.map((Object o) => '$o').toList());
  }
}
