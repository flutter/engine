// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

const String _green = '\u001b[1;32m';
const String _red = '\u001b[31m';
const String _gray = '\u001b[90m';
const String _reset = '\u001B[0m';

Future<void> step(String msg, Function() fn) async {
  stdout.writeln('-> $_green$msg$_reset');
  try {
    await fn();
  } finally {
    stdout.writeln('<- ${_gray}Done$_reset');
  }
}

void log(String msg) {
  stdout.writeln('$_gray$msg$_reset');
}

void panic(List<String> messages) {
  for (final String message in messages) {
    stderr.writeln('$_red$message$_reset');
  }
  throw 'panic';
}
