// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('ContextMenu', () {
    test('can enable and disable the context menu', () {
      expect(ContextMenu.contextMenuEnabled, isTrue);

      ContextMenu.disableContextMenu();
      expect(ContextMenu.contextMenuEnabled, isFalse);

      ContextMenu.disableContextMenu();
      expect(ContextMenu.contextMenuEnabled, isFalse);

      ContextMenu.enableContextMenu();
      expect(ContextMenu.contextMenuEnabled, isTrue);

      ContextMenu.enableContextMenu();
      expect(ContextMenu.contextMenuEnabled, isTrue);
    });
  });
}
