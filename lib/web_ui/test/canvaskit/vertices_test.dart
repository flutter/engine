// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:typed_data';

import 'package:test/test.dart';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

void main() {
  group(
    'SkVertices',
    () {
      setUpAll(() async {
        await ui.webOnlyInitializePlatform();
      });

      test('Using CanvasKit', () {
        expect(experimentalUseSkia, true);
      });

      test('Can create SkVertices.raw with null colors', () {
        SkVertices vertices =
            SkVertices.raw(ui.VertexMode.triangles, Float32List.fromList([]));
        expect(vertices, isNotNull);
      });
    },
  );
}
