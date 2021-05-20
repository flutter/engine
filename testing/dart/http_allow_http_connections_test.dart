// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9

import 'dart:async';
import 'dart:io';

import 'package:test/test.dart';

import 'http_disallow_http_connections_test.dart';
import 'test_util.dart';

main() {
  test('Normal HTTP request succeeds', () async {
    final host = await getLocalHostIP();
    await bindServerAndTest(host, (httpClient, uri) async {
      await httpClient.getUrl(uri);
    });
  });

  test('We can ban HTTP explicitly.', () async {
    final host = await getLocalHostIP();
    await bindServerAndTest(host, (httpClient, uri) async {
      await asyncExpectThrows<UnsupportedError>(
          () async => runZoned(() => httpClient.getUrl(uri),
            zoneValues: {#flutter.io.allow_http: false}));
    });
  });
}
