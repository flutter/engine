// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library mojom.command.single;

import 'dart:async';
import 'dart:io';

import 'package:args/args.dart';
import 'package:args/command_runner.dart';
import 'package:mojom/src/commands/mojom_command.dart';
import 'package:mojom/src/generate.dart';
import 'package:mojom/src/utils.dart';
import 'package:path/path.dart' as path;

class SinglePackageCommand extends MojomCommand {
  String get name => 'single';
  String get description => 'Generate bindings for a package.';
  String get invocation => 'mojom.dart single';

  Directory _package;

  SinglePackageCommand() {
    argParser.addOption('package',
        abbr: 'p',
        defaultsTo: Directory.current.path,
        help: 'Path to a Dart package.');
  }

  run() async {
    MojomCommand.setupLogging();
    await _validateArguments();

    var mojomFinder = new MojomFinder(mojomRoot, _package.parent, skips);
    List<PackageInfo> packageInfos = await mojomFinder.find();

    var generator = new MojomGenerator(mojoSdk,
        dryRun: dryRun, errorOnDuplicate: errorOnDuplicate);
    for (PackageInfo info in packageInfos) {
      await generator.generate(info);
    }
  }

  _validateArguments() async {
    await validateArguments();

    _package = new Directory(makeAbsolute(argResults['package']));
    log.info("package = $_package");
    if (!(await _package.exists())) {
      throw new CommandLineError("The package $_package must exist");
    }
  }
}
