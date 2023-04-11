// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/ui.dart';
import 'package:web_engine_tester/golden_tester.dart';

import 'utils.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUiTest();

  const Rect region = Rect.fromLTWH(0, 0, 300, 300);

  group('Linear Gradient', () {
    test('Using a linear gradient on a paint', () async {
      final PictureRecorder recorder = PictureRecorder();
      final Canvas canvas = Canvas(recorder, region);
      canvas.drawRect(
        const Rect.fromLTRB(50, 50, 250, 250),
        Paint()
          ..shader = Gradient.linear(
            const Offset(50, 50),
            const Offset(250, 250),
            <Color>[
              const Color(0xFFFF0000),
              const Color(0xFF00FF00),
              const Color(0xFF0000FF),
            ],
            <double>[0.0, 0.5, 1.0],
          )
          ..color = const Color(0XFF00FF00)
        );

      await drawPictureUsingCurrentRenderer(recorder.endRecording());

      await matchGoldenFile('linear_gradient_paint.png', region: region);
    });
  });
}
