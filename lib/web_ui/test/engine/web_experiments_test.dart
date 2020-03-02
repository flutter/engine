// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  tearDown(() {
    webExperiments.reset();
  });

  test('default web experiment values', () {
    expect(webExperiments.useSkia, false);
    expect(webExperiments.useCanvasText, false);
  });

  test('can turn on/off web experiments', () {
    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': true,
    });
    expect(webExperiments.useSkia, true);
    expect(webExperiments.useCanvasText, false);

    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': false,
      'useCanvasText': true,
    });
    expect(webExperiments.useSkia, false);
    expect(webExperiments.useCanvasText, true);

    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': true,
      'useCanvasText': null,
    });
    expect(webExperiments.useSkia, true);
    // Goes back to default value.
    expect(webExperiments.useCanvasText, false);

    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': null,
    });
    // Goes back to default value.
    expect(webExperiments.useSkia, false);
    expect(webExperiments.useCanvasText, false);
  });

  test('can reset web experiments', () {
    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': true,
      'useCanvasText': false,
    });
    webExperiments.reset();
    expect(webExperiments.useSkia, false);
    expect(webExperiments.useCanvasText, false);

    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': false,
      'useCanvasText': true,
    });
    webExperiments.reset();
    expect(webExperiments.useSkia, false);
    expect(webExperiments.useCanvasText, false);

    webExperiments.enableWebExperiments(<String, bool>{
      'useSkia': true,
      'useCanvasText': true,
    });
    webExperiments.reset();
    expect(webExperiments.useSkia, false);
    expect(webExperiments.useCanvasText, false);
  });
}
