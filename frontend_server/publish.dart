// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'package:path/path.dart' as path;

const String vmPubspec = r'''
name: vm
version: 0.0.1

dependencies:
  front_end: any
  kernel: any
  meta: any
  build_integratioon: any
''';

const String buildIntegrationPubspec = r'''
name: build_integration
version: 0.0.1

dependencies:
  front_end: any
  meta: any
''';

const Map<String, String> packages = <String, String>{
  'vm': vmPubspec,
  'build_integration': buildIntegrationPubspec,
};

// A script for creating a packagable version of several SDK libraries.
void main(List<String> arguments) {
  final Directory packageSource = Directory(arguments[0]);
  final String destination = arguments[1];
  final String packageName = arguments[2];
  for (File file in packageSource.listSync(recursive: true).whereType<File>()) {
    final File destinationFile = File(path.join(destination, path.relative(file.path, from: packageSource.path)));
    destinationFile.parent.createSync(recursive: true);
    file.copySync(destinationFile.path);
  }
  if (packages.containsKey(packageName)) {
    File(path.join(destination, 'pubspec.yaml'))
        .writeAsStringSync(packages[packageName]);
  }
}
