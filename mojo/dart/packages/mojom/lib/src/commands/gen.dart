// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library mojom.command.tree;

import 'dart:async';
import 'dart:io';

import 'package:args/args.dart';
import 'package:args/command_runner.dart';
import 'package:mojom/src/commands/mojom_command.dart';
import 'package:mojom/src/generate.dart';
import 'package:mojom/src/utils.dart';
import 'package:path/path.dart' as path;

class GenCommand extends MojomCommand {
  String get name => 'gen';
  String get description =>
      'Generate bindings for .mojom files under --mojom-root into Dart '
      'packages under --output';
  String get invocation => 'mojom.dart gen -r mojoms/ -o dart-packages/';

  Directory _dartRoot;

  GenCommand() {
    argParser.addOption('output',
        abbr: 'o',
        defaultsTo: Directory.current.path,
        help: 'Directory where Dart packages live into which we generate the '
            'resulting bindings.');
  }

  run() async {
    MojomCommand.setupLogging();
    await _validateArguments();
    var treeGenerator =
        new TreeGenerator(mojoSdk, mojomRoot, _dartRoot, skips, dryRun: dryRun);
    await treeGenerator.generate();
    return treeGenerator.errors;
  }

  _validateArguments() async {
    await validateArguments();
    _dartRoot = new Directory(makeAbsolute(argResults['output']));
    if (!await _dartRoot.exists()) {
      throw new CommandLineError(
          'Specified --output directory $_dartRoot does not exist');
    }
  }
}
