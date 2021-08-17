// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:path/path.dart' as path;

const List<String> basePathChunks = <String>[
  'gen',
  'flutter',
  'lib',
  'spirv',
  'test',
];

Directory spvDirectory(String leafFolderName) {
  assert(Platform.environment['FLUTTER_BUILD_DIRECTORY'] != null);

  return Directory(path.joinAll(<String>[
    ...path.split(Platform.environment['FLUTTER_BUILD_DIRECTORY']!),
    ...basePathChunks,
    leafFolderName,
  ])); 
}

File spvFile(String folderName, String fileName) {
  return File(path.joinAll(<String>[
    ...basePathChunks,
    folderName,
    fileName,
  ]));
}