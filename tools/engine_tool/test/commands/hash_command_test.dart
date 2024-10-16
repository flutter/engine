// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:ffi' as ffi show Abi;
import 'dart:io' as io;

import 'package:collection/collection.dart';
import 'package:engine_build_configs/engine_build_configs.dart';
import 'package:engine_repo_tools/engine_repo_tools.dart';
import 'package:engine_tool/src/commands/command_runner.dart';
import 'package:engine_tool/src/environment.dart';
import 'package:engine_tool/src/logger.dart';
import 'package:platform/platform.dart';
import 'package:process_fakes/process_fakes.dart';
import 'package:process_runner/process_runner.dart';
import 'package:test/test.dart';

void main() {
  final Engine engine;
  try {
    engine = Engine.findWithin();
  } catch (e) {
    io.stderr.writeln(e);
    io.exitCode = 1;
    return;
  }

  Environment linuxEnv(
      Logger logger, FakeProcessManager processManager, io.Stdout stdout) {
    return Environment(
      abi: ffi.Abi.linuxX64,
      engine: engine,
      stdout: stdout,
      platform: FakePlatform(
        resolvedExecutable: '/dart',
        operatingSystem: Platform.linux,
        pathSeparator: '/',
      ),
      processRunner: ProcessRunner(
        processManager: processManager,
      ),
      logger: logger,
    );
  }

  test('Producess an engine hash', () async {
    final fakeOut = FakeStdout();
    final logger = Logger.test((_) {});
    final manager = _fakeProcessManager(
      processes: <FakeProcess>[
        (exe: 'git', command: 'merge-base', rest: ['upstream/main', 'HEAD'], exitCode: 0, stdout: 'abcdef1234', stderr: null),
        (exe: 'git', command: 'ls-tree', rest: ['-r', 'abcdef1234'], exitCode: 0, stdout: 'one\r\ntwo\r\n', stderr: null),
      ],
    );
    final env = linuxEnv(logger, manager, fakeOut);
    final runner = ToolCommandRunner(
      environment: env,
      configs: <String, BuilderConfig>{},
    );
    final int result = await runner.run(<String>[
      'hash',
    ]);
    expect(result, equals(0));
    expect(fakeOut.writes, hasLength(1));
    expect(fakeOut.writes, [
      // sha1sum of "one\ntwo\n"
      'c708d7ef841f7e1748436b8ef5670d0b2de1a227',
    ]);
  });
}

typedef FakeProcess = ({
  String exe,
  String command,
  List<String> rest,
  dynamic stdout,
  dynamic stderr,
  int exitCode
});

FakeProcessManager _fakeProcessManager({
  required List<FakeProcess> processes,
}) {
  io.ProcessResult onRun(List<String> cmd) {
    for (final process in processes) {
      if (process.exe.endsWith(cmd[0]) &&
          process.command.endsWith(cmd[1]) &&
          process.rest.equals(cmd.sublist(2))) {
        return io.ProcessResult(
          1,
          process.exitCode,
          process.stdout,
          process.stderr,
        );
      }
    }
    return io.ProcessResult(
      1,
      -42,
      '',
      '404 command not found: $cmd',
    );
  }

  return FakeProcessManager(
    canRun: (Object? exe, {String? workingDirectory}) => true,
    onRun: onRun,
  );
}

class FakeStdout implements io.Stdout {
  FakeStdout();

  @override
  Encoding encoding = utf8;

  @override
  String lineTerminator = '\n';

  @override
  void add(List<int> data) {}

  @override
  void addError(Object error, [StackTrace? stackTrace]) {}

  @override
  // ignore: strict_raw_type
  Future addStream(Stream<List<int>> stream) async {
    throw UnimplementedError();
  }

  @override
  // ignore: strict_raw_type
  Future close() async {
    throw UnimplementedError();
  }

  @override
  // ignore: strict_raw_type
  Future get done => throw UnimplementedError();

  @override
  // ignore: strict_raw_type
  Future flush() {
    throw UnimplementedError();
  }

  @override
  bool get hasTerminal => throw UnimplementedError();

  @override
  io.IOSink get nonBlocking => throw UnimplementedError();

  @override
  bool get supportsAnsiEscapes => throw UnimplementedError();

  @override
  int get terminalColumns => throw UnimplementedError();

  @override
  int get terminalLines => throw UnimplementedError();

  final writes = <Object?>[];

  @override
  void write(Object? object) {
    writes.add(object);
  }

  @override
  // ignore: strict_raw_type
  void writeAll(Iterable objects, [String sep = '']) {
    writes.add((objects, sep));
  }

  @override
  void writeCharCode(int charCode) {
    writes.add(charCode);
  }

  @override
  void writeln([Object? object = '']) {
    writes.add((object, lineTerminator));
  }
}
