// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'browser.dart';
import 'safari_installation.dart';
import 'common.dart';

/// A class for running an instance of Safari.
///
/// Most of the communication with the browser is expected to happen via HTTP,
/// so this exposes a bare-bones API. The browser starts as soon as the class is
/// constructed, and is killed when [close] is called.
///
/// Any errors starting or running the process are reported through [onExit].
class Safari extends Browser {
  @override
  final name = 'Safari';

  static String version;

  /// Starts a new instance of Safari open to the given [url], which may be a
  /// [Uri] or a [String].
  factory Safari(Uri url, {bool debug = false}) {
    version = SafariArgParser.instance.version;

    assert(version != null);
    return Safari._(() async {
      // TODO(nurhan): Configure info log for LUCI.
      final BrowserInstallation installation = await getOrInstallSafari(
        version,
        infoLog: DevNull(),
      );

      // In the latest versions of MacOs opening Safari browser with a file brings
      // a popup which halts the test.
      // The following list of arguments needs to be provided to the `open` command
      // to open Safari for a given URL. In summary they provide a new instance
      // to open, that instance to wait for opening the url until Safari launches,
      // provide Safari bundles identifier.
      // For more details run `man open` on MacOS.
      // TODO(nurhan): explore implementing this part using Apple Script or
      // MacOS native APIs such as LSLaunchURLSpec. In the current solution there is
      // no way of closing Safari after opening with open command.
      var process = await Process.start(installation.executable, [
        '-F',
        '-W',
        '-n',
        '-b',
        'com.apple.Safari',
        '${url.toString()}'
      ] /* args */);

      return process;
    });
  }

  Safari._(Future<Process> startBrowser()) : super(startBrowser);
}
