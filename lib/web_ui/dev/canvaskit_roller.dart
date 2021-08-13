// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show json;
import 'dart:io';

import 'package:path/path.dart' as pathlib;
import 'package:yaml/yaml.dart';

import 'environment.dart';
import 'utils.dart';

/// Fetches a snapshot of the pre-built CanvasKit from the CDN using the version
/// specified in `dev/canvaskit_lock.yaml`, and packages it into a CIPD package.
///
/// Verifies that the version is correctly specified in the `DEPS` file and in
/// `lib/web_ui/lib/src/engine/canvaskit/initialization.dart`.
Future<void> main(List<String> args) async {
  final String canvaskitVersion = _readCanvaskitVersion();
  await _validateCanvaskitInitializationCode(canvaskitVersion);

  final Directory canvaskitDirectory = await Directory.systemTemp.createTemp('canvaskit-roll-$canvaskitVersion-');
  print('Will use ${canvaskitDirectory.path} as staging directory.');

  final String baseUrl = 'https://unpkg.com/canvaskit-wasm@$canvaskitVersion/bin/';
  print('Downloading CanvasKit from $baseUrl');
  final HttpClient client = HttpClient();
  for (final String assetPath in _canvaskitAssets) {
    final String assetUrl = '$baseUrl/$assetPath';
    final File assetFile = File(pathlib.joinAll(<String>[
      canvaskitDirectory.path,
      'canvaskit',
      ...assetPath.split('/'), // so it's compatible with Windows
    ]));
    await assetFile.parent.create(recursive: true);
    final HttpClientRequest request = await client.getUrl(Uri.parse(assetUrl));
    final HttpClientResponse response = await request.close();
    final IOSink fileSink = assetFile.openWrite();
    await response.pipe(fileSink);
  }
  client.close();

  final File cipdConfigFile = File(pathlib.join(
    canvaskitDirectory.path,
    'cipd.yaml',
  ));
  await cipdConfigFile.writeAsString('''
package: flutter/web/canvaskit_bundle
description: A build of CanvasKit bundled with Flutter Web apps
data:
  - dir: canvaskit
''');

  print('Uploading to CIPD');
  await runProcess('cipd', <String>[
    'create',
    '--tag=version:$canvaskitVersion',
    '--pkg-def=cipd.yaml',
    '--json-output=result.json',
  ], workingDirectory: canvaskitDirectory.path);

  final Map<String, dynamic> cipdResult = json.decode(File(pathlib.join(
    canvaskitDirectory.path,
    'result.json',
  )).readAsStringSync()) as Map<String, dynamic>;
  final String cipdInstanceId = cipdResult['result']['instance_id'] as String;

  print('CIPD instance information:');
  final String cipdInfo = await evalProcess('cipd', <String>[
    'describe',
    'flutter/web/canvaskit_bundle',
    '--version=$cipdInstanceId',
  ], workingDirectory: canvaskitDirectory.path);
  print(cipdInfo.trim().split('\n').map((String line) => ' • $line').join('\n'));

  print('Updating DEPS file');
  await _updateDepsFile(cipdInstanceId);

  print('\nATTENTION: the roll process is not complete yet.');
  print('Last step: for the roll to take effect submit an engine pull request from local git changes.');
}

const List<String> _canvaskitAssets = <String>[
  'canvaskit.js',
  'canvaskit.wasm',
  'profiling/canvaskit.js',
  'profiling/canvaskit.wasm',
];

String _readCanvaskitVersion() {
  final YamlMap canvaskitLock = loadYaml(File(pathlib.join(
    environment.webUiDevDir.path,
    'canvaskit_lock.yaml',
  )).readAsStringSync()) as YamlMap;
  return canvaskitLock['canvaskit_version'] as String;
}

Future<void> _updateDepsFile(String cipdInstanceId) async {
  final File depsFile = File(pathlib.join(
    environment.flutterDirectory.path,
    'DEPS',
  ));

  final String originalDepsCode = await depsFile.readAsString();
  final List<String> rewrittenDepsCode = <String>[];
  const String kCanvasKitDependencyKeyInDeps = '\'canvaskit_cipd_instance\': \'';
  bool canvaskitDependencyFound = false;
  for (final String line in originalDepsCode.split('\n')) {
    if (line.trim().startsWith(kCanvasKitDependencyKeyInDeps)) {
      canvaskitDependencyFound = true;
      rewrittenDepsCode.add(
        "  'canvaskit_cipd_instance': '$cipdInstanceId',",
      );
    } else {
      rewrittenDepsCode.add(line);
    }
  }

  if (!canvaskitDependencyFound) {
    stderr.writeln(
      'Failed to update the DEPS file.\n'
      'Could not to locate CanvasKit dependency in the DEPS file. Make sure the '
      'DEPS file contains a line like this:\n'
      '\n'
      '  \'canvaskit_cipd_instance\': \'SOME_VALUE\','
    );
    exit(1);
  }

  await depsFile.writeAsString(rewrittenDepsCode.join('\n'));
}

Future<void> _validateCanvaskitInitializationCode(String canvaskitVersion) async {
  const String kCanvasKitVersionKey = 'const String canvaskitVersion';
  const String kPathToInitializationCode = 'lib/src/engine/canvaskit/initialization.dart';
  final String initializationCode = await File(pathlib.join(
    environment.webUiRootDir.path,
    kPathToInitializationCode,
  )).readAsString();
  final String versionInCode = initializationCode
    .split('\n')
    .firstWhere(
      (String line) => line.startsWith(kCanvasKitVersionKey),
      orElse: () {
        throw Exception('Failed to find "$kCanvasKitVersionKey" in $kPathToInitializationCode');
      }
    );

  final String expectedVersion = '$kCanvasKitVersionKey = \'$canvaskitVersion\';';
  if (versionInCode != expectedVersion) {
    stderr.writeln(
      'ERROR: the CanvasKit bundle version specified in $kPathToInitializationCode '
      'does not match the version specified in lib/web_ui/dev/canvaskit_lock.yaml.\n'
      '\n'
      'Expected: $expectedVersion\n'
      'Actual  : $versionInCode\n'
      '\n'
      'To fix the issue, edit $kPathToInitializationCode and update the value of '
      'the constant. Alternatively, revert the changes made in '
      'lib/web_ui/dev/canvaskit_lock.yaml.',
    );
    exit(1);
  }
}
