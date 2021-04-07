// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:io';
import 'dart:typed_data';

import 'package:spirv/spirv.dart';
import 'package:path/path.dart' as path;

class Target {
  final TargetLanguage language;
  final String extension;

  const Target._(this.language, this.extension);
}

const sksl = Target._(TargetLanguage.sksl, '.sksl.golden');
const glslES = Target._(TargetLanguage.glslES, '.glslES.golden');
const glslES300 = Target._(TargetLanguage.glslES300, '.glslES300.golden');

/// All compilation targets and their extensions.
const targets = <Target>[glslES, glslES300, sksl];

typedef GoldenHandler = Future<void> Function(String path, ByteBuffer spirv);

Stream<File> goldens() async* {
  final dir = Directory(path.join('test', 'goldens'));
  await for (final entry in dir.list()) {
    if (entry is! File) {
      continue;
    }
    final file = entry as File;
    if (path.extension(file.path) != '.spv') {
      continue;
    }
    yield file;
  }
}
