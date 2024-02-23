// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Note that this file doesn't live in a "traditional" pub package.
import 'package:args/args.dart';

void main(List<String> args) {
  final ArgParser argParser = ArgParser()
    ..addFlag(
      'help',
      abbr: 'h',
      help: 'Print this help message.',
      negatable: false,
    );
  final ArgResults argResults = argParser.parse(args);
  if (argResults['help'] as bool) {
    print(argParser.usage);
    return;
  }
  print('Hello, world!');
}
