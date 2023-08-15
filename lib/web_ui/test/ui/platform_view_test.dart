// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:ui/ui_web/src/ui_web.dart' as ui_web;
import 'package:web_engine_tester/golden_tester.dart';

import '../common/test_initialization.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUnitTests(
    emulateTesterEnvironment: false,
    setUpTestViewDimensions: false,
  );

  const ui.Rect region = ui.Rect.fromLTWH(0, 0, 300, 300);
  const String platformViewType = 'test-platform-view';

  setUp(() {
    ui_web.platformViewRegistry.registerViewFactory(
      platformViewType,
      (int viewId) { 
        final DomElement element = createDomHTMLDivElement();
        element.style.backgroundColor = 'blue';
        return element;
      }
    );
  });

  tearDown(() {
    platformViewManager.debugClear();
  });

  test('Picture + Overlapping PlatformView', () async {
    await _createPlatformView(1, platformViewType);

    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Canvas canvas = ui.Canvas(recorder);
    canvas.drawCircle(
      ui.Offset.zero,
      50,
      ui.Paint()
        ..style = ui.PaintingStyle.fill
        ..color = const ui.Color(0xFFFF0000)
    );

    final ui.SceneBuilder sb = ui.SceneBuilder();
    sb.pushOffset(0, 0);
    sb.addPicture(const ui.Offset(150, 150), recorder.endRecording());

    sb.addPlatformView(
      1,
      offset: const ui.Offset(125, 125),
      width: 50,
      height: 50,  
    );
    await renderer.renderScene(sb.build());

    await matchGoldenFile('picture_platformview_overlap.png', region: region);
  });

  test('PlatformView sandwich', () async {
    await _createPlatformView(1, platformViewType);

    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Canvas canvas = ui.Canvas(recorder);
    canvas.drawCircle(
      ui.Offset.zero,
      50,
      ui.Paint()
        ..style = ui.PaintingStyle.fill
        ..color = const ui.Color(0xFF00FF00)
    );

    final ui.Picture picture = recorder.endRecording();

    final ui.SceneBuilder sb = ui.SceneBuilder();
    sb.pushOffset(0, 0);
    sb.addPicture(const ui.Offset(125, 125), picture);

    sb.addPlatformView(
      1,
      offset: const ui.Offset(150, 150),
      width: 50,
      height: 50,
    );

    sb.addPicture(const ui.Offset(175, 175), picture);
    await renderer.renderScene(sb.build());

    await matchGoldenFile('picture_platformview_sandwich.png', region: region);
  });
}

// Sends a platform message to create a Platform View with the given id and viewType.
Future<void> _createPlatformView(int id, String viewType) {
  final Completer<void> completer = Completer<void>();
  const MethodCodec codec = StandardMethodCodec();
  window.sendPlatformMessage(
    'flutter/platform_views',
    codec.encodeMethodCall(MethodCall(
      'create',
      <String, dynamic>{
        'id': id,
        'viewType': viewType,
      },
    )),
    (dynamic _) => completer.complete(),
  );
  return completer.future;
}
