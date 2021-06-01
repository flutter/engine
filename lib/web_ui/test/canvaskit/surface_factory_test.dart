// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:html' as html;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import '../matchers.dart';
import 'common.dart';

const MethodCodec codec = StandardMethodCodec();

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$SurfaceFactory', () {
    setUpCanvasKitTest();

    setUp(() {
      window.debugOverrideDevicePixelRatio(1);
    });

    test('cannot be created with size less than 2', () {
      expect(() => SurfaceFactory(-1), throwsAssertionError);
      expect(() => SurfaceFactory(0), throwsAssertionError);
      expect(() => SurfaceFactory(1), throwsAssertionError);
      expect(SurfaceFactory(2), isNotNull);
    });

    test('the number of canvases on-screen will never exceed the maximum',
        () async {
      expect(SurfaceFactory.instance.debugCacheSize, 0);
      final CkPicture testPicture =
          paintPicture(ui.Rect.fromLTRB(0, 0, 10, 10), (CkCanvas canvas) {
        canvas.drawCircle(ui.Offset(5, 5), 5, CkPaint());
      });

      // Initialize all platform views to be used in the test.
      final List<int> platformViewIds = <int>[];
      for (int i = 0; i < HtmlViewEmbedder.maximumOverlaySurfaces * 2; i++) {
        ui.platformViewRegistry.registerViewFactory(
          'test-platform-view',
          (viewId) => html.DivElement()..id = 'view-$i',
        );
        await _createPlatformView(i, 'test-platform-view');
        platformViewIds.add(i);
      }

      final EnginePlatformDispatcher dispatcher =
          ui.window.platformDispatcher as EnginePlatformDispatcher;

      void renderTestScene({required int viewCount}) {
        LayerSceneBuilder sb = LayerSceneBuilder();
        sb.pushOffset(0, 0);
        for (int i = 0; i < viewCount; i++) {
          sb.addPicture(ui.Offset.zero, testPicture);
          sb.addPlatformView(i, width: 10, height: 10);
        }
        dispatcher.rasterizer!.draw(sb.build().layerTree);
      }

      int countCanvases() {
        return domRenderer.sceneElement!.querySelectorAll('canvas').length;
      }

      // Frame 1:
      //   Render: Twice the maximum number of overlay surfaces
      //   Expect: Maximum number of canvases
      renderTestScene(viewCount: HtmlViewEmbedder.maximumOverlaySurfaces * 2);
      expect(countCanvases(), HtmlViewEmbedder.maximumOverlaySurfaces);

      // Frame 2:
      //   Render: zero platform views.
      //   Expect: main canvas, no overlays; overlays in the cache.
      await Future<void>.delayed(Duration.zero);
      renderTestScene(viewCount: 0);
      expect(countCanvases(), 1);
      // The cache contains all the surfaces except the base surface and the
      // backup surface.
      expect(SurfaceFactory.instance.debugCacheSize,
          HtmlViewEmbedder.maximumOverlaySurfaces - 2);

      // Frame 3:
      //   Render: less than cache size platform views.
      //   Expect: overlays reused; cache shrinks.
      await Future<void>.delayed(Duration.zero);
      renderTestScene(viewCount: HtmlViewEmbedder.maximumOverlaySurfaces - 2);
      expect(countCanvases(), HtmlViewEmbedder.maximumOverlaySurfaces - 1);
      expect(SurfaceFactory.instance.debugCacheSize, 0);

      // Frame 4:
      //   Render: more platform views than max cache size.
      //   Expect: main canvas, backup overlay, maximum overlays;
      //           cache empty (everything reused).
      await Future<void>.delayed(Duration.zero);
      renderTestScene(viewCount: HtmlViewEmbedder.maximumOverlaySurfaces * 2);
      expect(countCanvases(), HtmlViewEmbedder.maximumOverlaySurfaces);
      expect(SurfaceFactory.instance.debugCacheSize, 0);

      // Frame 5:
      //   Render: zero platform views.
      //   Expect: main canvas, no overlays; cache full but does not exceed limit.
      await Future<void>.delayed(Duration.zero);
      renderTestScene(viewCount: 0);
      expect(countCanvases(), 1);
      expect(SurfaceFactory.instance.debugCacheSize,
          HtmlViewEmbedder.maximumOverlaySurfaces - 2);

      // Frame 6:
      //   Render: deleted platform views.
      //   Expect: error.
      for (final int id in platformViewIds) {
        final codec = StandardMethodCodec();
        final Completer<void> completer = Completer<void>();
        ui.window.sendPlatformMessage(
          'flutter/platform_views',
          codec.encodeMethodCall(MethodCall(
            'dispose',
            id,
          )),
          completer.complete,
        );
        await completer.future;
      }

      try {
        renderTestScene(viewCount: platformViewIds.length);
        fail('Expected to throw');
      } on AssertionError catch (error) {
        expect(
          error.toString(),
          'Assertion failed: "Cannot render platform views: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15. These views have not been created, or they have been deleted."',
        );
      }

      // Frame 7:
      //   Render: a platform view after error.
      //   Expect: success. Just checking the system is not left in a corrupted state.
      await _createPlatformView(0, 'test-platform-view');
      renderTestScene(viewCount: 0);
    });

    // TODO: https://github.com/flutter/flutter/issues/60040
  }, skip: isIosSafari);
}

// Sends a platform message to create a Platform View with the given id and viewType.
Future<void> _createPlatformView(int id, String viewType) {
  final completer = Completer<void>();
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
