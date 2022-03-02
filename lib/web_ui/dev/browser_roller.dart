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

final ArgParser _argParser = ArgParser(allowTrailingOptions: false)
  ..addOption(
    'version-tag',
    abbr: 't',
    help: 'The tag to assign to the uploaded package. This label can be used '
          'to refer to this version from .ci.yaml.',
    mandatory: true,
    valueHelp: 'VERSION'
  )..addOption(
    'platform',
    abbr: 'p',
    // See _platformBindings below
    allowed: <String>['linux', 'mac', 'windows'],
    help: 'The platform to download. If not passed, the script will download '
          'for all available platforms. One of: ',
    mandatory: false,
    valueHelp: 'PLATFORM',
  )..addFlag(
    'dry-run',
    defaultsTo: false,
    help: 'Whether or not to push changes to CIPD. When --dry-run is set, the '
          'script will download everything and attempt to prepare the bundle '
          'but will stop before publishing. When not set, the bundle will be '
          'published.',
    negatable: false,
  )..addFlag(
    'verbose',
    abbr: 'v',
    defaultsTo: false,
    help: 'Enable verbose output.',
    negatable: false,
  );

late final String versionTag;
late final String? desiredPlatform;
late final bool dryRun;
late final bool verbose;

final Client _client = Client();

/// Rolls browser CIPD packages to the version specified in `browser_lock.yaml`.
///
/// Currently only rolls Chrome.
///
/// These rolls are consumed by the "chrome_and_driver" LUCI recipe, here:
/// https://cs.opensource.google/flutter/recipes/+/main:recipe_modules/flutter_deps/api.py;l=146
Future<void> main(List<String> args) async {
  try {
    processArgs(_argParser.parse(args));
    await _BrowserRoller().roll();
    io.exitCode = 0;
  } on FormatException catch (e) {
    print('''
Error! ${e.message}

Available options:

${_argParser.usage}
''');
    io.exitCode = 1;
  } finally {
    _client.close();
  }
}

// Initialize globals from the parsed command-line arguments.
void processArgs(ArgResults args) {
  versionTag = args['version-tag'] as String;
  desiredPlatform = args['platform'] as String?;
  dryRun = args['dry-run'] as bool;
  verbose = args['verbose'] as bool;
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

  // Prints output when --verbose is set.
  void vprint(String out) {
    if (verbose) {
      print(out);
    }
  }

  // Roll Chromium and ChromeDriver for each of the Platforms.
  Future<void> roll() async {
    for (final MapEntry<String, PlatformBinding> entry in _platformBindings.entries) {
      final String platform = entry.key;
      if (desiredPlatform != null && platform != desiredPlatform) {
        vprint('Skipped platform: $platform');
        continue;
      }
      final PlatformBinding binding = entry.value;
      await _rollChromium(platform, binding);
      await _rollChromeDriver(platform, binding);
    }
    if (dryRun) {
      print('\nDry Run! Non-published roll artifacts kept here: ${_rollDir.path}\n');
    } else {
      // Clean-up
      vprint('\nDeleting temporary directory: ${_rollDir.path}');
      await _rollDir.delete(recursive: true);
      print('\nDone.\n');
    }
  }

  // Returns the contents for the CIPD config required to publish a new chromium package.
  String _getCipdChromiumConfig({
    required String platform,
    required String version,
    required String build,
    required String root,
  }) {
    return '''
package: flutter_internal/browsers/chrome/$platform-amd64
description: Chromium $version (build $build) used for testing
preserve_writable: true
root: $root
data:
  - dir: .
''';
  }

  // Returns the contents for the CIPD config required to publish a new chromedriver package.
  String _getCipdChromedriverConfig({
    required String platform,
    required String version,
    required String build,
    required String root,
  }) {
    return '''
package: flutter_internal/browser-drivers/chrome/$platform-amd64
description: Chromedriver for Chromium $version (build $build) used for testing
preserve_writable: true
root: $root
data:
  - dir: .
''';
  }

  // Download a file from the internet, and put it in a temporary location.
  Future<io.File> _downloadTemporaryFile(String url) async {
    // Use the hash of the Url to temporarily store a file under tmp
    final io.File downloadedFile = io.File(path.join(
        io.Directory.systemTemp.path,
        'download_' + url.hashCode.toRadixString(16),
      ));
    vprint('  Downloading [$url] into [${downloadedFile.path}]');
    final StreamedResponse download = await _client.send(
      Request('GET', Uri.parse(url)),
    );
    await download.stream.pipe(downloadedFile.openWrite());
    return downloadedFile;
  }

  // Unzips a `file` into a `destination` Directory (must exist).
  Future<void> _unzipAndDeleteFile(io.File zipFile, io.Directory destination) async {
    vprint('  Unzipping [${zipFile.path}] into [$destination]');
    await runProcess('unzip', <String>[
      if (!verbose) ...<String>[
        '-q',
      ],
      zipFile.path,
      '-d',
      destination.path,
    ]);
    vprint('  Deleting [${zipFile.path}]');
    await zipFile.delete();
  }

  // Write String `contents` to a file in `path`.
  //
  // This is used to write CIPD config files to disk.
  Future<io.File> _writeFile(String path, String contents) async {
    vprint('  Writing file [$path]');
    final io.File file = io.File(path, );
    await file.writeAsString(contents);
    return file;
  }

  // Locate the first subdirectory that contains more than one file under `root`.
  // (or one ".app" bundle for mac)
  //
  // When uncompressing files, unzip might create some extra directories, but it
  // seems that our scripts want our CIPD packages to contain everything in the root.
  Future<io.Directory?> _locateContentRoot(io.Directory root) async {
    final List<io.FileSystemEntity> children = root.listSync(followLinks: false);
    assert(children.isNotEmpty);
    if (root.path.toLowerCase().endsWith('.app')) {
      // We've gone inside the .app bundle of the mac version!
      return root.parent;
    }
    if (children.length == 1) {
      if (children.first is io.Directory) {
        return _locateContentRoot(children.first as io.Directory);
      } else {
        return root;
      }
    }
    return root;
  }

  // Runs a CIPD command to upload a package defined by its `config` file.
  Future<int> _uploadToCipd({required io.File config}) {
    final String cipdCommand = dryRun ? 'pkg-build' : 'create';
    // CIPD won't fully shut up even in 'error' mode
    final String logLevel = verbose ? 'debug' : 'warning';
    vprint('  Running CIPD $cipdCommand');
    return runProcess('cipd', <String>[
      'pkg-build', // TODO(dit): use cipdCommand
      '--pkg-def',
      path.basename(config.path),
      '--json-output',
      path.basenameWithoutExtension(config.path)+'.json',
      '--log-level',
      logLevel,
      if (!dryRun) ...<String>[
        '--tag',
        versionTag,
      ],
      if (dryRun) ...<String>[
        '--out',
        path.basenameWithoutExtension(config.path)+'.zip',
      ],
    ], workingDirectory: _rollDir.path);
  }

  // Downloads Chromium from the internet, packs it in the directory structure
  // that the LUCI script wants. The result of this will be then uploaded to CIPD.
  Future<void> _rollChromium(String platform, PlatformBinding binding) async {
    final String chromeBuild = binding.getChromeBuild(_lock.chromeLock);
    final String url = binding.getChromeDownloadUrl(chromeBuild);
    final io.Directory platformDir = io.Directory(path.join(_rollDir.path, platform));
    await platformDir.create(recursive: true);

    print('\nRolling Chromium for $platform (build $chromeBuild)');
    vprint('  Created target directory [${platformDir.path}]');

    final io.File chromeDownload = await _downloadTemporaryFile(url);

    await _unzipAndDeleteFile(chromeDownload, platformDir);

    // Zips might create spurious subdirs, find them and take them into account for our root CIPD param
    final io.Directory? actualContentRootDir = await _locateContentRoot(platformDir);
    assert(actualContentRootDir != null);
    final String relativePlatformDir = path.relative(actualContentRootDir!.path, from: _rollDir.path);

    // Create the config manifest to upload to CIPD
    final String cipdConfigFilePath = path.join(_rollDir.path, 'cipd.chromium.$platform.yaml');
    final io.File cipdConfigFile = await _writeFile(
        cipdConfigFilePath,
        _getCipdChromiumConfig(
            platform: platform,
            version: versionTag,
            build: chromeBuild,
            root: relativePlatformDir,
        ));
    // Run CIPD
    await _uploadToCipd(config: cipdConfigFile);
  }

  // Downloads Chromedriver from the internet, packs it in the directory structure
  // that the LUCI script wants. The result of this will be then uploaded to CIPD.
  Future<void> _rollChromeDriver(String platform, PlatformBinding binding) async {
    final String chromeBuild = binding.getChromeBuild(_lock.chromeLock);
    final String url = binding.getChromeDriverDownloadUrl(chromeBuild);
    final io.Directory platformDir = io.Directory(path.join(_rollDir.path, '${platform}_driver'));
    await platformDir.create(recursive: true);

    print('\nRolling Chromedriver for $platform (build $chromeBuild)');
    vprint('  Created target directory [${platformDir.path}]');

    final io.File chromedriverDownload = await _downloadTemporaryFile(url);

    await _unzipAndDeleteFile(chromedriverDownload, platformDir);

    // Zips might create spurious subdirs, find them and take them into account for our root CIPD param
    final io.Directory? actualContentRootDir = await _locateContentRoot(platformDir);
    assert(actualContentRootDir != null);
    final String relativePlatformDir = path.relative(actualContentRootDir!.path, from: _rollDir.path);

    // Create the config manifest to upload to CIPD
    final String cipdConfigFilePath = path.join(_rollDir.path, 'cipd.chromedriver.$platform.yaml');
    final io.File cipdConfigFile = await _writeFile(
        cipdConfigFilePath,
        _getCipdChromedriverConfig(
            platform: platform,
            version: versionTag,
            build: chromeBuild,
            root: relativePlatformDir,
        ));
    // Run CIPD
    await _uploadToCipd(config: cipdConfigFile);
  }
}
