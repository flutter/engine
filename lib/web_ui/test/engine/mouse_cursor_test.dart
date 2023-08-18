// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$MouseCursor', () {
    test('sets correct `cursor` style on root element', () {
      final DomElement rootViewElement = createDomElement('div');
      final MockFlutterView view = MockFlutterView()
        ..rootElement = rootViewElement;
      final MouseCursor mouseCursor = MouseCursor(view);

      mouseCursor.activateSystemCursor('alias');
      expect(rootViewElement.style.cursor, 'alias');

      mouseCursor.activateSystemCursor('move');
      expect(rootViewElement.style.cursor, 'move');

      mouseCursor.activateSystemCursor('precise');
      expect(rootViewElement.style.cursor, 'crosshair');

      mouseCursor.activateSystemCursor('resizeDownRight');
      expect(rootViewElement.style.cursor, 'se-resize');
    });

    test('handles unknown cursor type', () {
      final DomElement rootViewElement = createDomElement('div');
      final MockFlutterView view = MockFlutterView()
        ..rootElement = rootViewElement;
      final MouseCursor mouseCursor = MouseCursor(view);

      mouseCursor.activateSystemCursor('unknown');
      expect(rootViewElement.style.cursor, 'default');
    });
  });
}

class MockFlutterView implements EngineFlutterView {
  @override
  late DomElement rootElement;

  @override
  double get devicePixelRatio => throw UnimplementedError();

  @override
  ui.Display get display => throw UnimplementedError();

  @override
  List<ui.DisplayFeature> get displayFeatures => throw UnimplementedError();

  @override
  ui.GestureSettings get gestureSettings => throw UnimplementedError();

  @override
  MouseCursor get mouseCursor => throw UnimplementedError();

  @override
  ViewPadding get padding => throw UnimplementedError();

  @override
  ui.Rect get physicalGeometry => throw UnimplementedError();

  @override
  ui.Size get physicalSize => throw UnimplementedError();

  @override
  ui.PlatformDispatcher get platformDispatcher => throw UnimplementedError();

  @override
  void render(ui.Scene scene) {
    throw UnimplementedError();
  }

  @override
  ViewPadding get systemGestureInsets => throw UnimplementedError();

  @override
  void updateSemantics(ui.SemanticsUpdate update) {
    throw UnimplementedError();
  }

  @override
  int get viewId => throw UnimplementedError();

  @override
  ViewPadding get viewInsets => throw UnimplementedError();

  @override
  ViewPadding get viewPadding => throw UnimplementedError();
}
