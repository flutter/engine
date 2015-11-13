// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library mojom.command_runner;

import 'dart:async';
import 'dart:io';

import 'package:args/args.dart';
import 'package:args/command_runner.dart';
import 'package:mojom/src/commands/check.dart';
import 'package:mojom/src/commands/gen.dart';
import 'package:mojom/src/commands/single.dart';

class MojomCommandRunner extends CommandRunner {
  MojomCommandRunner()
      : super("mojom", "mojom is a tool for managing Mojo bindings for Dart.") {
    super.argParser.addFlag('dry-run',
        abbr: 'd',
        defaultsTo: false,
        negatable: false,
        help: 'Print the operations that would have been run, but'
            'do not run anything.');
    super.argParser.addFlag('ignore-duplicates',
        abbr: 'i',
        defaultsTo: false,
        negatable: false,
        help: 'Ignore generation of a .mojom.dart file into the same location '
            'as an existing file. By default this is an error');
    super.argParser.addOption('mojom-root',
        abbr: 'r',
        defaultsTo: Directory.current.path,
        help: 'Directory from which to begin the search for .mojom files if '
            'needed.');
    super.argParser.addOption('mojo-sdk',
        abbr: 'm',
        defaultsTo: Platform.environment['MOJO_SDK'],
        help: 'Path to the Mojo SDK, which can also be specified '
            'with the environment variable MOJO_SDK.');
    super.argParser.addOption('skip',
        abbr: 's', allowMultiple: true, help: 'Directories to skip.');
    super.argParser.addFlag('verbose',
        abbr: 'v',
        defaultsTo: false,
        negatable: false,
        help: 'Show extra output about what mojom is doing.');

    super.addCommand(new CheckCommand());
    super.addCommand(new SinglePackageCommand());
    super.addCommand(new GenCommand());
  }
}
