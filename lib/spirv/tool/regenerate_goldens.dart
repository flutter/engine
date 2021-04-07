// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:async';
import 'dart:io';

import 'package:path/path.dart' as path;
import 'package:spirv/spirv.dart';

import '../test/golden.dart';

void main() async {
  await for (final file in goldens()) {
    final bytes = file.readAsBytesSync().buffer;
    for (final target in targets) {
      final result = transpile(bytes, target.language);
      final out = File(path.join(
        'test',
        'goldens',
        path.basenameWithoutExtension(file.path) + target.extension,
      ));
      out.writeAsStringSync(result.source);
    }
  }
}
