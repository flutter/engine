// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:js_interop';
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:ui/ui_web/src/ui_web.dart' as ui_web;
import 'package:web_engine_tester/golden_tester.dart';

@JS('window.flutterConfiguration')
external set _jsConfiguration(JsFlutterConfiguration? configuration);

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  test('fonts can be loaded manually', () async {
    _jsConfiguration = null;

    // Fully initialize engine services and UI without manifest fonts. Test that
    // the engine can still render everything, albeit without text.
    await initializeEngineServices(jsConfiguration: JsFlutterConfiguration(
      loadManifestFontsBeforeAppMain: false.toJS,
      canvasKitBaseUrl: '/canvaskit/'.toJS,
    ));
    await initializeEngineUi();

    String? messageChannel;
    Object? messageData;

    EnginePlatformDispatcher.instance.onPlatformMessage = (String name, ByteData? data, ui.PlatformMessageResponseCallback? callback) {
      messageChannel = name;
      messageData = const JSONMessageCodec().decodeMessage(data);
    };

    await _testTextRendering('canvaskit_text_without_fonts');
    expect(messageChannel, isNull);
    expect(messageData, isNull);

    // Load fonts manually. Test that text is now rendered fully.
    await ui_web.loadManifestFonts();

    // The notification message is sent in the next frame so there's only one
    // notification for a batch of fonts, so wait for 2 frames before checking
    // for the message.
    final Completer<void> waitForFontMessage = Completer<void>();
    domWindow.requestAnimationFrame((_) {
      domWindow.requestAnimationFrame((_) {
        waitForFontMessage.complete();
      });
    });
    await waitForFontMessage.future;

    expect(messageChannel, 'flutter/system');
    expect(messageData, <String, dynamic>{'type': 'fontsChange'});
    await _testTextRendering('canvaskit_text_with_fonts');
  });
}

Future<void> _testTextRendering(String testName) async {
  final CkPicture picture;
  {
    final CkPictureRecorder recorder = CkPictureRecorder();
    final CkCanvas canvas = recorder.beginRecording(ui.Rect.largest);
    final CkParagraphBuilder builder = CkParagraphBuilder(CkParagraphStyle());
    builder.addText('Hello');
    final CkParagraph paragraph = builder.build();
    paragraph.layout(const ui.ParagraphConstraints(width: 1000));
    canvas.drawRect(
      ui.Rect.fromLTWH(0, 0, paragraph.width, paragraph.height).inflate(10),
      CkPaint()..color = const ui.Color(0xFF00FF00)
    );
    canvas.drawParagraph(paragraph, ui.Offset.zero);
    picture = recorder.endRecording();
  }

  {
    final LayerSceneBuilder builder = LayerSceneBuilder();
    builder.pushOffset(0, 0);
    builder.addPicture(ui.Offset.zero, picture);
    final LayerTree layerTree = builder.build().layerTree;
    CanvasKitRenderer.instance.rasterizer.draw(layerTree);

    await matchGoldenFile('$testName.png',
        region: const ui.Rect.fromLTRB(0, 0, 50, 30));
  }
}
