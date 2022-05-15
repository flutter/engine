// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:process/process.dart';

extension RunAndForward on ProcessManager {
  /// Runs [cmd], and forwards the stdout and stderr pipes to the parent process.
  Future<int> runAndForward(List<String> cmd) async {
    final Process process = await start(cmd);
    final Completer<void> stdoutCompleter = Completer<void>();
    final StreamSubscription<String> stdoutSub = process.stdout
      .transform(utf8.decoder)
      .transform<String>(const LineSplitter())
      .listen((String line) {
        stdout.writeln('[stdout] $line');
      }, onDone: stdoutCompleter.complete);

    final Completer<void> stderrCompleter = Completer<void>();
    final StreamSubscription<String> stderrSub = process.stderr
      .transform(utf8.decoder)
      .transform<String>(const LineSplitter())
      .listen((String line) {
        stdout.writeln('[stderr] $line');
      }, onDone: stderrCompleter.complete);

    final int exitCode = await process.exitCode;
    await stdoutCompleter.future;
    await stderrCompleter.future;

    stderrSub.cancel();
    stdoutSub.cancel();
    return exitCode;
  }
}
