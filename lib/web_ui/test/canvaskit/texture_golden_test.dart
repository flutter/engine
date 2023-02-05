// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:web_engine_tester/golden_tester.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('CanvasKit Texture', () {
    setUpCanvasKitTest();

    setUp(() {
      window.debugOverrideDevicePixelRatio(1);
    });

    test('can take an image as a texture', () async {
      final DomHTMLImageElement image = createDomHTMLImageElement()
        ..src = '/test_images/mandrill_128.png';

      await _waitImageLoaded(image);

      final int textureId = ui.textureRegistry.registerTexture(image);

      final LayerSceneBuilder sb = LayerSceneBuilder();
      sb.pushOffset(0, 0);
      sb.addTexture(textureId, width: 50, height: 50);

      CanvasKitRenderer.instance.rasterizer.draw(sb.build().layerTree);
      await matchGoldenFile(
        'canvaskit_texture_image.png',
        region: const ui.Rect.fromLTRB(0, 0, 128, 128),
      );
    });

    test('can mark a texture as updated', () async {
      final DomHTMLImageElement image = createDomHTMLImageElement()
        ..src = '/test_images/mandrill_128.png';

      await _waitImageLoaded(image);

      final DomCanvasElement canvas = createDomCanvasElement(
        width: 128,
        height: 128,
      );

      final int textureId = ui.textureRegistry.registerTexture(canvas);

      final LayerSceneBuilder sb = LayerSceneBuilder();
      sb.pushOffset(0, 0);
      sb.addTexture(textureId, width: 128, height: 128);

      canvas.context2D.drawImage(image, 0, 0);
      ui.textureRegistry.textureFrameAvailable(textureId);
      CanvasKitRenderer.instance.rasterizer.draw(sb.build().layerTree);

      canvas.context2D.drawImage(image, 50, 50);
      ui.textureRegistry.textureFrameAvailable(textureId);
      CanvasKitRenderer.instance.rasterizer.draw(sb.build().layerTree);

      await matchGoldenFile(
        'canvaskit_texture_canvas.png',
        region: const ui.Rect.fromLTRB(0, 0, 128, 128),
      );
    });
  });
}

Future<void> _waitImageLoaded(DomHTMLImageElement image) {
  final Completer<void> imageLoaded = Completer<void>();

  image.addEventListener('load', allowInterop((_) {
    imageLoaded.complete();
  }));

  return imageLoaded.future;
}
