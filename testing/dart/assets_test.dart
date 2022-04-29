// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui' as ui;

import 'package:litetest/litetest.dart';

void main() {
  test('Loading an asset that does not exist returns null', () {
    expect(ui.ImmutableBuffer.fromAsset('ThisDoesNotExist'), null);
  });

  test('returns the bytes of a bundled asset', () {
    expect(ui.ImmutableBuffer.fromAsset('FontManifest.json'), isNotNull);
  });
}
