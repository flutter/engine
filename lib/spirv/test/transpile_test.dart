// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:io';
import 'dart:typed_data';

import 'package:path/path.dart' as path;
import 'package:test/test.dart';
import 'package:spirv/spirv.dart';

import 'golden.dart';

void main() {
  test('match goldens', () async {
    await for (final file in goldens()) {
      for (final target in targets) {
        final source = File(path.join(
          path.dirname(file.path),
          path.basenameWithoutExtension(file.path) + target.extension,
        ));
        if (!source.existsSync()) {
          fail('${file.path} does not have matching golden'); 
        }
        final result = transpile(
          file.readAsBytesSync().buffer,
          target.language,
        );
        expect(result.source, equals(source.readAsStringSync()));
      }
    }
  });
}
