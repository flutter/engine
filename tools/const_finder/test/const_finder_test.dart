// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show jsonEncode;
import 'dart:io';

import 'package:const_finder/const_finder.dart';
import 'package:path/path.dart' as path;

int exitCode = 0;

void expect<T>(T value, T expected) {
  if (value != expected) {
    stderr.writeln('Expected: $expected');
    stderr.writeln('Actual:   $value');
    exitCode = -1;
  }
}

final String basePath =
    path.canonicalize(path.join(path.dirname(Platform.script.path), '..'));
final String fixtures = path.join(basePath, 'test', 'fixtures');
final String consts = path.join(fixtures, 'lib', 'consts.dart');
final String constsAndNon = path.join(fixtures, 'lib', 'consts_and_non.dart');
final String constsDill = path.join(fixtures, 'consts.dill');
final String constsAndNonDill = path.join(fixtures, 'consts_and_non.dill');

// This test is assuming the `dart` used to invoke the tests is compatible
// with the version of package:kernel in //third-party/dart/pkg/kernel
final String dart = Platform.resolvedExecutable;
final String bat = Platform.isWindows ? '.bat' : '';
final String pub = path.join(path.dirname(dart), 'pub$bat');

_checkConsts() {
  print('Checking for expected constants.');
  final ConstFinder finder = ConstFinder(
    kernelFilePath: constsDill,
    targetLibraryUri: 'package:const_finder_fixtures/consts.dart',
    classLibraryUri: 'package:const_finder_fixtures/target.dart',
    className: 'Target',
  );

  expect<String>(
    jsonEncode(finder.findInstances()),
    jsonEncode(<String, dynamic>{
      'constantInstances': <Map<String, dynamic>>[
        <String, dynamic>{'stringValue': '1', 'intValue': 1},
        <String, dynamic>{'stringValue': '2', 'intValue': 2}
      ],
      'nonConstantLocations': <dynamic>[],
    }),
  );
}

_checkNonConsts() {
  print('Checking for non-constant instances.');
  final ConstFinder finder = ConstFinder(
    kernelFilePath: constsAndNonDill,
    targetLibraryUri: 'package:const_finder_fixtures/consts_and_non.dart',
    classLibraryUri: 'package:const_finder_fixtures/target.dart',
    className: 'Target',
  );

  expect<String>(
    jsonEncode(finder.findInstances()),
    jsonEncode(<String, dynamic>{
      'constantInstances': <dynamic>[
        <String, dynamic>{'stringValue': '1', 'intValue': 1}
      ],
      'nonConstantLocations': <dynamic>[
        <String, dynamic>{
          'file': 'file://$fixtures/lib/consts_and_non.dart',
          'line': 12,
          'column': 26
        },
        <String, dynamic>{
          'file': 'file://$fixtures/lib/consts_and_non.dart',
          'line': 14,
          'column': 26
        },
      ]
    }),
  );
}

Future<void> main() async {
  try {
    print('Getting packages...');
    ProcessResult result = Process.runSync(
      pub,
      <String>['get', '--offline'], // This doesn't retrieve any packages
      workingDirectory: fixtures,
    );
    expect<int>(result.exitCode, 0);
    print('Generating kernel fixtures...');
    result = Process.runSync(dart, <String>[
      '--snapshot-kind=kernel',
      '--snapshot=$constsDill',
      consts,
    ]);
    expect<int>(result.exitCode, 0);
    result = Process.runSync(dart, <String>[
      '--snapshot-kind=kernel',
      '--snapshot=$constsAndNonDill',
      constsAndNon,
    ]);
    expect(result.exitCode, 0);

    _checkConsts();
    _checkNonConsts();
  } finally {
    try {
      File(constsDill).deleteSync();
      File(constsAndNonDill).deleteSync();
    } finally {
      print('Tests ${exitCode == 0 ? 'succeeded' : 'failed'} - exit code: $exitCode');
      exit(exitCode);
    }
  }
}
