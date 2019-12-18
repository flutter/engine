// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:test/test.dart';

import 'browser.dart';
import 'safari.dart';
import 'test_platform.dart';

void main() async {
  test('Start Safari Browser', () async {
    Browser browser = Safari(Uri.parse('https://www.google.com'), debug: false);

    var completer = Completer<BrowserManager>();

    browser.onExit.then((_) {
      throw Exception('Safari exited before connecting.');
    }).catchError((error, StackTrace stackTrace) {
      if (completer.isCompleted) return;
      completer.completeError(error, stackTrace);
    });
  });
}
