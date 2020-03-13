// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:html' as html;
import 'dart:js_util' as js_util;

import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  tearDown(() {
    webExperiments.reset();
  });

  test('default web experiment values', () {
    expect(webExperiments.useCanvasText, false);
  });

  test('can turn on/off web experiments', () {
    webExperiments.updateExperiment('useCanvasText', true);
    expect(webExperiments.useCanvasText, true);

    webExperiments.updateExperiment('useCanvasText', false);
    expect(webExperiments.useCanvasText, false);

    webExperiments.updateExperiment('useCanvasText', null);
    // Goes back to default value.
    expect(webExperiments.useCanvasText, false);
  });

  test('ignores unknown experiments', () {
    expect(webExperiments.useCanvasText, false);
    webExperiments.updateExperiment('foobarbazqux', true);
    expect(webExperiments.useCanvasText, false);
    webExperiments.updateExperiment('foobarbazqux', false);
    expect(webExperiments.useCanvasText, false);
  });

  test('can reset web experiments', () {
    webExperiments.updateExperiment('useCanvasText', true);
    webExperiments.reset();
    expect(webExperiments.useCanvasText, false);

    webExperiments.updateExperiment('useCanvasText', true);
    webExperiments.updateExperiment('foobarbazqux', true);
    webExperiments.reset();
    expect(webExperiments.useCanvasText, false);
  });

  test('js interop also works', () {
    expect(webExperiments.useCanvasText, false);

    js_util.callMethod(
      html.window,
      '_flutter_internal_update_experiment',
      <dynamic>['useCanvasText', true],
    );
    expect(webExperiments.useCanvasText, true);

    js_util.callMethod(
      html.window,
      '_flutter_internal_update_experiment',
      <dynamic>['useCanvasText', null],
    );
    expect(webExperiments.useCanvasText, false);
  });
}
