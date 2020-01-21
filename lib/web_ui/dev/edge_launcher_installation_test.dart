// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@TestOn('vm && windows')

import 'dart:io' as io;

import 'package:test/test.dart';

import 'edge_installation.dart';

void main() async {
  void deleteEdgeInstallIfExists() {
    final io.Directory installationDirectory =
        EdgeLauncher().launcherInstallationDir;
    if (installationDirectory.existsSync()) {
      installationDirectory.deleteSync(recursive: true);
    }
  }

  setUpAll(() {
    deleteEdgeInstallIfExists();
  });

  tearDown(() {
    deleteEdgeInstallIfExists();
  });

  test('installs a given version of Firefox', () async {
    final EdgeLauncher edgeLauncher = EdgeLauncher();

    expect(edgeLauncher.isInstalled, isFalse);

    await getEdgeInstallation('system');

    expect(EdgeLauncher().launcherInstallationDir.existsSync(), isTrue);
    expect(edgeLauncher.isInstalled, isTrue);
  });
}
