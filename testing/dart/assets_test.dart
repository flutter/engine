// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:ui' as ui;

import 'package:test/test.dart';

void main() {
  test('Loading an asset that does not exist returns null', () {
    expect(ui.loadAsset('ThisDoesNotExist'), null);
  });

  test('returns the bytes of a bundled asset', () {
    expect(ui.loadAsset('FontManifest.json'), isNotNull);
  });
}
