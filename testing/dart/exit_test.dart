// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:litetest/litetest.dart';

/// Verifies that io.exit() throws an exception.
void main() {
  test('dart:io exit() throws an exception', () async {
    bool caught = false;
    try {
      io.exit(0);
    } on UnsupportedError catch (e) {
      expect(e.toString(), contains("disallows calling dart:io's exit()"));
      caught = true;
    }
    expect(caught, true);
  });
}
