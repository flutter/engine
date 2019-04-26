// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data' show Float64List;
import 'dart:ui';

import 'package:test/test.dart';

void main() {
  test('pushTransform throws with invalid matrx', () {
    final SceneBuilder builder = SceneBuilder();
    final Float64List matrix4 = Float64List(16);
    matrix4[0] = double.nan;
    matrix4[1] = double.infinity;
    expect(
      () => builder.pushTransform(matrix4),
      throwsA(TypeMatcher<ArgumentError>())
    );
  });
}
