// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:mojom/src/command_runner.dart';
import 'package:mojom/src/utils.dart';
import 'package:path/path.dart' as path;
import 'package:unittest/unittest.dart';

final singlePacakgeMojomContents = '''
[DartPackage="single_package"]
module single_package;
struct SinglePackage {
  int32 thingo;
};
''';

Future runCommand(List<String> args) {
  return new MojomCommandRunner().run(args);
}

main() async {
  String mojoSdk;
  if (Platform.environment['MOJO_SDK'] != null) {
    mojoSdk = Platform.environment['MOJO_SDK'];
  } else {
    mojoSdk = path.normalize(path.join(
        path.dirname(Platform.script.path), '..', '..', '..', '..', 'public'));
  }
  if (!await new Directory(mojoSdk).exists()) {
    fail("Could not find the Mojo SDK");
  }

  final scriptPath = path.dirname(Platform.script.path);

  // //test_mojoms/mojom
  final testMojomPath = path.join(scriptPath, 'test_mojoms');

  setUp(() async {
    await new Directory(testMojomPath).create(recursive: true);
  });

  tearDown(() async {
    await new Directory(testMojomPath).delete(recursive: true);
  });

  group('Commands', () {
    // //single_package
    final singlePackagePath = path.join(scriptPath, 'single_package');
    // //single_package/.mojoms
    final singlePackageMojomsPath = path.join(singlePackagePath, '.mojoms');
    // //single_package/lib
    final singlePackageLibPath = path.join(singlePackagePath, 'lib');
    // //single_package/packages
    final singlePackagePackagesPath = path.join(singlePackagePath, 'packages');
    // //single_package/packages/single_package
    final singlePackagePackagePath =
        path.join(singlePackagePackagesPath, 'single_package');

    setUp(() async {
      await new Directory(singlePackageLibPath).create(recursive: true);
      await new Directory(singlePackagePackagesPath).create(recursive: true);
      await new Link(singlePackagePackagePath).create(singlePackageLibPath);

      // //test_mojoms/single_package/public/interfaces/single_package.mojom
      final singlePackageMojomFile = new File(path.join(testMojomPath,
          'single_package', 'public', 'interfaces', 'single_package.mojom'));
      await singlePackageMojomFile.create(recursive: true);
      await singlePackageMojomFile.writeAsString(singlePacakgeMojomContents);
    });

    tearDown(() async {
      await new Directory(singlePackagePath).delete(recursive: true);
    });

    test('single', () async {
      await runCommand([
        'single',
        '-m',
        mojoSdk,
        '-r',
        testMojomPath,
        '-p',
        singlePackagePath
      ]);

      // Should have:
      // //single_package/lib/single_package/single_package.mojom.dart
      final resultPath = path.join(
          singlePackageLibPath, 'single_package', 'single_package.mojom.dart');
      final resultFile = new File(resultPath);
      expect(await resultFile.exists(), isTrue);

      // There should be no stray .mojoms file haning around.
      final mojomsFile = new File(singlePackageMojomsPath);
      expect(await mojomsFile.exists(), isFalse);
    });

    test('gen', () async {
      await runCommand(
          ['gen', '-m', mojoSdk, '-r', testMojomPath, '-o', scriptPath]);

      // Should have:
      // //single_package/lib/single_package/single_package.mojom.dart
      final resultPath = path.join(
          singlePackageLibPath, 'single_package', 'single_package.mojom.dart');
      final resultFile = new File(resultPath);
      expect(await resultFile.exists(), isTrue);

      // There should be no stray .mojoms file haning around.
      final mojomsFile = new File(singlePackageMojomsPath);
      expect(await mojomsFile.exists(), isFalse);
    });
  });
}
