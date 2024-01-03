// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => doTests);
}

void doTests() {
  group('GlobalHtmlAttributes', () {
    test('applies global attributes to the global element', () {
      final DomElement globalElement = createDomElement('global-element');
      final GlobalHtmlAttributes globalHtmlAttributes = GlobalHtmlAttributes(globalElement);

      globalHtmlAttributes.applyAttributes(
        autoDetectRenderer: true,
        rendererTag: 'canvaskit',
        buildMode: 'release',
      );

      expect(globalElement.getAttribute('flt-renderer'), 'canvaskit (auto-selected)');
      expect(globalElement.getAttribute('flt-build-mode'), 'release');
      expect(globalElement.getAttribute('spellcheck'), 'false');

      globalHtmlAttributes.applyAttributes(
        autoDetectRenderer: false,
        rendererTag: 'html',
        buildMode: 'debug',
      );

      expect(globalElement.getAttribute('flt-renderer'), 'html (requested explicitly)');
      expect(globalElement.getAttribute('flt-build-mode'), 'debug');
      expect(globalElement.getAttribute('spellcheck'), 'false');
    });
  });
}
