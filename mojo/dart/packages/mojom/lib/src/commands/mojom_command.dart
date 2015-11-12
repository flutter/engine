// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library mojom.command;

import 'dart:async';
import 'dart:io';

import 'package:args/args.dart';
import 'package:args/command_runner.dart';
import 'package:logging/logging.dart' as logging;
import 'package:mojom/src/utils.dart';
import 'package:path/path.dart' as path;

abstract class MojomCommand extends Command {
  bool _verbose;
  bool _dryRun;
  bool _errorOnDuplicate;
  Directory _mojomRoot;
  Directory _mojoSdk;
  List<String> _skips;

  bool get verbose => _verbose;
  bool get dryRun => _dryRun;
  bool get errorOnDuplicate => _errorOnDuplicate;
  Directory get mojomRoot => _mojomRoot;
  Directory get mojoSdk => _mojoSdk;
  List<String> get skips => _skips;

  static setupLogging() {
    if (log == null) {
      logging.hierarchicalLoggingEnabled = true;
      log = new logging.Logger('mojom');
      log.onRecord.listen((logging.LogRecord rec) {
        print('${rec.level.name}: ${rec.message}');
      });
    }
  }

  validateArguments() async {
    assert(log != null);
    if (globalResults['verbose']) {
      log.level = logging.Level.INFO;
    } else {
      log.level = logging.Level.WARNING;
    }
    _dryRun = globalResults['dry-run'];
    _errorOnDuplicate = !globalResults['ignore-duplicates'];

    final mojoSdkPath = globalResults['mojo-sdk'];
    if (mojoSdkPath == null) {
      throw new CommandLineError(
          "The Mojo SDK directory must be specified with the --mojo-sdk "
          "flag or the MOJO_SDK environment variable.");
    }
    _mojomRoot = new Directory(makeAbsolute(globalResults['mojom-root']));
    _mojoSdk = new Directory(makeAbsolute(mojoSdkPath));
    log.info("Mojo SDK = $_mojoSdk");
    if (!(await _mojoSdk.exists())) {
      throw new CommandLineError(
          "The specified Mojo SDK directory $_mojoSdk must exist.");
    }
    if (globalResults['skip'] != null) {
      _skips = globalResults['skip'].map(makeAbsolute).toList();
    }
  }

  String toString() {
    String dart = makeRelative(Platform.executable);
    String script = makeRelative(path.fromUri(Platform.script));
    String globals = globalResults.arguments.join(" ");
    return "$dart $script $globals";
  }
}
