// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:io';
import 'dart:typed_data';

import 'package:spirv/spirv.dart';

import '../test/golden.dart';

/// A simple benchmark to test how quickly the golden files
/// at test/goldens can be transpiled.
///
/// Accepts a single optional integer argument for how many
/// times to perform each transpilation, and it will return
/// the average of those values.
///
/// The default number of iterations is tuned for running as
/// an AOT compiled binary with
/// `dart compile exe benchmark/benchmark_goldens.dart`
///
/// When benchmarking with the dart VM, low iteration values
/// will result in VM 'warm-up' having a disproportionate
/// effect on the results.
void main(List<String> args) async {
  int iterations = 1000;
  if (args.isNotEmpty) {
    iterations = int.parse(args[0]);
  }
  print('Benchmarking with $iterations iterations');
  print('');
  await for (final file in goldens()) {
    print('${file.path}:');
    final spirv = file.readAsBytesSync().buffer;
    for (final target in targets) {
      double runtime = 0;
      for (int i = 0; i < iterations; i++) {
        final sw = Stopwatch()..start();
        transpile(spirv, target.language);
        sw.stop();
        runtime += sw.elapsedMicroseconds;
      }
      runtime /= iterations;
      String lang = target.language.toString().split('.').last + ':';
      lang = lang.padRight(15);
      print('$lang $runtime\u{00b5}s');
    }
    print('');
  }
}
