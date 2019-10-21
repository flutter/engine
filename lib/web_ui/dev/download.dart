// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:args/command_runner.dart';

import 'common.dart';
import 'firefox_installer.dart';

class DownloadCommand extends Command<bool> {
  DownloadCommand();

  @override
  String get name => 'download';

  @override
  String get description => 'Deletes build caches and artifacts.';

  @override
  FutureOr<bool> run() async {
    BrowserInstallation installation = await getOrInstallFirefox('69.0.2');

    print('Firefox installed: ${installation.version}');
    print('Firefox installed: ${installation.executable}');
  }
}
