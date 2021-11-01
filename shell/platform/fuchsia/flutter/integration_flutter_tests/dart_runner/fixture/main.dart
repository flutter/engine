// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:core';
import 'dart:fuchsia' as fuchsia;
import 'dart:io';

// ignore_for_file: dead_code

Future<List<String>> run(String command) async {
  try {
    final List<String> splitCommand = command.split(' ');
    final String exe = splitCommand[0];
    final List<String> args = splitCommand.skip(1).toList();
    // This needs to remain async in the event that this command attempts to
    // access something (like the hub) that requires interaction with this
    // process's event loop. A specific example is attempting to run `find`, a
    // synchronous command, on this own process's `out` directory. As `find`
    // will wait indefinitely for the `out` directory to be serviced, causing
    // a deadlock.
    final ProcessResult r = await Process.run(exe, args);
    return (r.stdout as String).split('\n');
  } on ProcessException catch (e) {
    print("Error running '$command': $e");
  }
  return <String>[];
}

void main(List<String> args) async {
  String test_case = args.isEmpty ? "" : args[0];

  if (test_case == "StartupShutdown") {
    fuchsia.exit(42);
  } else if (test_case == "RunProcess") {
    await run('/bin/ls /bin');
  }

  fuchsia.exit(0);
}
