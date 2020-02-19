// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io' as io;

import 'package:args/args.dart';

import 'common.dart';

class SafariArgParser extends BrowserArgParser {
  static final SafariArgParser _singletonInstance = SafariArgParser._();

  /// The [SafariArgParser] singleton.
  static SafariArgParser get instance => _singletonInstance;

  String _version;

  SafariArgParser._();

  @override
  void populateOptions(ArgParser argParser) {
    argParser
      ..addOption(
        'safari-version',
        defaultsTo: 'system',
        help: 'The Safari version to use while running tests. The Safari '
            'browser installed on the system is used as the only option now.'
            'Soon we will add support for using different versions using the '
            'tech previews.',
      );
  }

  @override
  void parseOptions(ArgResults argResults) {
    _version = argResults['safari-version'];
    assert(_version == 'system');
  }

  @override
  String get version => _version;
}

/// Returns the installation of Safari.
///
/// Currently uses the Safari version installed on the operating system.
///
/// Latest Safari version for Catalina, Mojave, High Siera is 13.
///
/// Latest Safari version for Sierra is 12.
// TODO(nurhan): user latest version to download and install the latest
// technology preview.
Future<BrowserInstallation> getOrInstallSafari(
  String requestedVersion, {
  StringSink infoLog,
}) async {

  // These tests are aimed to run only on MacOs machines local or on LUCI.
  if (!io.Platform.isMacOS) {
    throw UnimplementedError('Safari on ${io.Platform.operatingSystem} is'
        ' not supported. Safari is only supported on MacOS.');
  }

  infoLog ??= io.stdout;

  if (requestedVersion == 'system') {
    // Since Safari is included in MacOS, always assume there will be one on the
    // system.
    infoLog.writeln('Using the system version that is already installed.');
    printSafariVersion(infoLog);
    return BrowserInstallation(
      version: 'system',
      executable: PlatformBinding.instance.getMacApplicationLauncher(),
    );
  } else {
    infoLog.writeln('Unsupported version $requestedVersion.');
    throw UnimplementedError();
  }
}

/// Since differnt Safari versions can create different results for tests,
/// it is useful to log this information for debug purposes.
Future<void> printSafariVersion(StringSink infoLog) async {
  final io.ProcessResult safariVersionResult =
    await io.Process.run('system_profiler', <String>['SPApplicationsDataType']);

  if(safariVersionResult.exitCode !=0) {
    infoLog.writeln('Safari version not available');
  } else {
    final String output = safariVersionResult.stdout;

    // The output is information about all the applications.
    final List<String> listOfResults = output.split('\n');

    // Find the line which contains version info for Safari.
    int locationForSafariVersion = 0;
    bool safariFound = false;
    for (int i=0; i<listOfResults.length; i++) {
      if (safariFound) {
        if (listOfResults[i].contains('Version:')) {
          locationForSafariVersion = i;
          break;
        }
      }
      if (listOfResults[i].contains('Safari:')) {
        safariFound = true;
      }
    }

    // The version line should look like: `Version:  13.0.5.`
    final String versionLine = listOfResults[locationForSafariVersion];
    final String version = versionLine.substring(versionLine.indexOf(':')+2);
    infoLog.writeln('Safari version in use $version.');
    print('Safari version in use $version.');
  }
}
