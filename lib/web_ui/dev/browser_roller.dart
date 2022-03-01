// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:http/http.dart';
import 'package:path/path.dart' as path;

import 'browser_lock.dart';
import 'common.dart';
import 'utils.dart';

final ArgParser _argParser = ArgParser()
  ..addOption(
    'version-tag',
    defaultsTo: null,
    mandatory: true,
    help: 'The tag to assign to the uploaded package. This label can be used '
          'to refer to this version from .ci.yaml.',
  );

late final String versionTag;
final Client _client = Client();

/// Rolls browser CIPD packages to the version specified in `browser_lock.yaml`.
///
/// Currently only rolls Chrome.
Future<void> main(List<String> args) async {
  processArgs(_argParser.parse(args));
  await _BrowserRoller().roll();
  _client.close();
}

void processArgs(ArgResults args) {
  versionTag = args['version-tag'] as String;
}

class _BrowserRoller {
  _BrowserRoller();

  final io.Directory _rollDir = io.Directory.systemTemp.createTempSync('browser-roll-');

  final Map<String, PlatformBinding> _platformBindings = <String, PlatformBinding>{
    'linux': LinuxPlatformBinding(),
    'mac': MacPlatformBinding(),
    'windows': WindowsPlatformBinding(),
  };

  final BrowserLock _lock = BrowserLock();

  Future<void> roll() async {
    print('Staging directory for the roll is ${_rollDir.path}');
    for (final MapEntry<String, PlatformBinding> entry in _platformBindings.entries) {
      final String platform = entry.key;
      final PlatformBinding binding = entry.value;
      await _rollChrome(platform, binding);
    }
  }

  // TODO(yjbanov): Also roll the corresponding chromedriver
  Future<void> _rollChrome(String platform, PlatformBinding binding) async {
    print('Rolling Chromium for $platform');
    final String chromeBuild = binding.getChromeBuild(_lock.chromeLock);
    final String url = binding.getChromeDownloadUrl(chromeBuild);
    print(' Fetching $chromeBuild from $url');
    final StreamedResponse download = await _client.send(
      Request('GET', Uri.parse(url)),
    );

    final io.Directory platformDir = io.Directory(path.join(_rollDir.path, platform));
    await platformDir.create(recursive: true);
    final io.File downloadedFile = io.File(path.join(platformDir.path, 'chrome.zip'));
    await download.stream.pipe(downloadedFile.openWrite());
    print('  Unzipping archive into a local staging directory');
    await runProcess('unzip', <String>[
      downloadedFile.path,
      '-d',
      platformDir.path,
    ]);

    final String cipdConfigFileName = 'cipd.$platform.yaml';
    final io.File cipdConfigFile = io.File(path.join(
      _rollDir.path,
      cipdConfigFileName,
    ));
    await cipdConfigFile.writeAsString('''
package: flutter_internal/browsers/chrome/$platform-amd64
description: Chromium $chromeBuild used for testing
preserve_writable: true
data:
  - dir: ${path.relative(platformDir.path, from: _rollDir.path)}
''');

    print('Uploading to CIPD');
    print('DRY RUN:\n${cipdConfigFile.readAsStringSync()}');
    // await runProcess('cipd', <String>[
    //   'create',
    //   '--tag=$versionTag',
    //   '--pkg-def=$cipdConfigFileName',
    //   '--json-output=result.json',
    // ], workingDirectory: _rollDir.path);
  }
}
