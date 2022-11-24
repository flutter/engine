// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$CkTextureRegistry', () {
    setUpCanvasKitTest();

    setUp(() {
      window.debugOverrideDevicePixelRatio(1);
    });

    test('can register and unregister a texture', () async {
      final CkTextureRegistry registry = CanvasKitRenderer.instance.textureRegistry;

      final DomHTMLImageElement image = createDomHTMLImageElement();
      final int id = registry.registerTexture(image);

      expect(id, isNotNull);
      expect(registry.getTexture(id), isNotNull);

      registry.unregisterTexture(id);
      expect(registry.getTexture(id), isNull);
    });

    test('can register multiple textures with different ids', () async {
      final CkTextureRegistry registry = CanvasKitRenderer.instance.textureRegistry;

      final DomHTMLImageElement image1 = createDomHTMLImageElement();
      final int id1 = registry.registerTexture(image1);

      final DomHTMLImageElement image2 = createDomHTMLImageElement();
      final int id2 = registry.registerTexture(image2);

      expect(id1, isNotNull);
      expect(id2, isNotNull);
      expect(id2, isNot(id1));

      registry.unregisterTexture(id1);
      expect(registry.getTexture(id1), isNull);

      final DomHTMLImageElement image3 = createDomHTMLImageElement();
      final int id3 = registry.registerTexture(image3);

      expect(id3, isNotNull);
      expect(id3, isNot(id1));
      expect(id3, isNot(id2));
    });
  });
}
