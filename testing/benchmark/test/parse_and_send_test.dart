// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:test/test.dart';

void main() {
  test('parse_and_send with example json does not crash.', () async {
    final String testCred =
        File('secret/test_gcp_credentials.json').readAsStringSync();
    Process.runSync('dart', <String>[
      'bin/parse_and_send.dart',
      'example/txt_benchmarks.json',
    ], environment: <String, String>{
      'BENCHMARK_GCP_CREDENTIALS': testCred,
    });
  });
}
