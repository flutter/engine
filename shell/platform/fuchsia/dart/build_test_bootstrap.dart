// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:args/args.dart';
import 'package:flutter_tools/src/test/flutter_platform.dart' as loader;

const String _testNameKey = 'test-name';
const String _outputKey = 'output';

/// Builds the Flutter test wrapper that gets executed by the test harness.
Future<Null> main(List<String> args) async {
  ArgParser parser = ArgParser();
  parser.addOption(
    _testNameKey,
    valueHelp: 'filename',
    help: 'Basename of the test script file being wrapped.',
  );
  parser.addOption(
    _outputKey,
    valueHelp: 'path',
    help: 'Path to the output file that this tool should generate.',
  );
  ArgResults results = parser.parse(args);

  if (!results.wasParsed(_outputKey) || !results.wasParsed(_testNameKey)) {
    stderr.writeln(parser.usage);
    exit(1);
  }

  String content = loader.generateTestBootstrap(
    testUrl: Uri.parse(results[_testNameKey]),
    host: InternetAddress.loopbackIPv4,
  );

  File outputFile = File(results[_outputKey]);
  await outputFile.writeAsString(content);
}
