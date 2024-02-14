// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// #############################################################################
// This is a script that will let you check golden image diffs locally.
//
// Usage: compare_goldens.dart <dir path> <dir path>
//
// The directories are scanned for png files that match in name, then the diff
// is written to `diff_<name of file>` in the CWD. This allows you to get
// results quicker than having to upload to skia gold.  By default it uses fuzzy
// RMSE to compare.
//
// Here's the steps for using this with something like impeller golden tests:
// 1) Checkout a base revision
// 2) Build impeller_golden_tests
// 3) Execute `impeller_golden_tests --working_dir=<path a>
// 4) Checkout test revision
// 5) Build impeller_golden_tests
// 6) Execute `impeller_golden_tests --working_dir=<path b>
// 7) Execute `compare_goldens.dart <path a> <path b>
//
// Requirements: ImageMagick is installed on $PATH
// #############################################################################

import 'dart:io';

bool hasCommandOnPath(String name) {
  final ProcessResult result = Process.runSync('which', <String>[name]);
  return result.exitCode == 0;
}

List<String> findPairs(Set<String> as, Set<String> bs) {
  final List<String> result  = <String>[];
  for (final String a in as) {
    if (bs.contains(a)) {
      result.add(a);
    } else {
      print('Mix match file $a.');
      exitCode = 1;
    }
  }
  for (final String b in bs) {
    if (!as.contains(b)) {
      print('Mix match file $b.');
      exitCode = 1;
    }
  }
  return result;
}

String basename(String path) {
  return path.split(Platform.pathSeparator).last;
}

Set<String> grabPngFilenames(Directory dir) {
  return dir.listSync()
    .map((e) => basename(e.path))
    .where((e) => e.endsWith('.png'))
    .toSet();
}

void main(List<String> args) {
  if (!hasCommandOnPath('compare')) {
    throw Exception(r'Could not find `compare` from ImageMagick on $PATH.');
  }
  if (args.length != 2) {
    throw Exception('Usage: compare_goldens.dart <dir path> <dir path>');
  }

  final Directory dirA = Directory(args[0]);
  if (!dirA.existsSync()) {
    throw Exception('Unable to find $dirA');
  }
  final Directory dirB = Directory(args[1]);
  if (!dirB.existsSync()) {
    throw Exception('Unable to find $dirB');
  }

  final Set<String> filesA = grabPngFilenames(dirA);
  final Set<String> filesB = grabPngFilenames(dirB);
  final List<String> pairs = findPairs(filesA, filesB);

  int count = 0;
  for (final String name in pairs) {
    count += 1;
    final String pathA = <String>[dirA.path, name].join(Platform.pathSeparator);
    final String pathB = <String>[dirB.path, name].join(Platform.pathSeparator);
    final String output = 'diff_$name';
    print('compare ($count / ${pairs.length}) $name');
    final ProcessResult result = Process.runSync('compare', ['-metric', 'RMSE', '-fuzz', '5%', pathA, pathB, output]);
    if (result.exitCode != 0) {
      print('DIFF FOUND: saved to $output');
      exitCode = 1;
    } else {
      File(output).deleteSync();
    }
  }
}
