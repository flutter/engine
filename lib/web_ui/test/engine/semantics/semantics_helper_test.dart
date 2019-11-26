// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:js_util' as js_util;

import 'package:ui/src/engine.dart';

import 'package:test/test.dart';

void main() {
  group('$DesktopEnableSemantics', () {
    DesktopEnableSemantics desktopEnableSemantics;
    html.Element _placeholder;

    setUp(() {
      desktopEnableSemantics = DesktopEnableSemantics();
    });

    tearDown(() {
      if (_placeholder != null) {
        _placeholder.remove();
      }
      if(desktopEnableSemantics?.semanticsActivationTimer != null) {
        desktopEnableSemantics.semanticsActivationTimer.cancel();
        desktopEnableSemantics.semanticsActivationTimer = null;
      }
    });

    test('prepare accesibility placeholder', () async {
      _placeholder = desktopEnableSemantics.prepareAccesibilityPlaceholder();

      expect(_placeholder.getAttribute('role'), 'button');
      expect(_placeholder.getAttribute('aria-live'), 'true');
      expect(_placeholder.getAttribute('tabindex'), '0');

      html.document.body.append(_placeholder);

      expect(html.document.getElementsByTagName('flt-semantics-placeholder'),
          isNotEmpty);

      expect(_placeholder.getBoundingClientRect().height, 1);
      expect(_placeholder.getBoundingClientRect().width, 1);
      expect(_placeholder.getBoundingClientRect().top, -1);
      expect(_placeholder.getBoundingClientRect().left, -1);
    });

    test('Not relevant events should be forwarded to the framework', () async {
      // Prework. Attach the placeholder to dom.
      _placeholder = desktopEnableSemantics.prepareAccesibilityPlaceholder();
      html.document.body.append(_placeholder);

      html.Event event = dispatchMouseEvent('mousemove');
      bool shouldForwardToFramework =
          desktopEnableSemantics.tryEnableSemantics(event);

      expect(shouldForwardToFramework, true);

      event = dispatchPointerEvent('pointermove');
      shouldForwardToFramework =
          desktopEnableSemantics.tryEnableSemantics(event);

      expect(shouldForwardToFramework, true);
    });

    test(
        'Relevants events targeting placeholder should not be forwarded to the framework',
        () async {
      // Prework. Attach the placeholder to dom.
      _placeholder = desktopEnableSemantics.prepareAccesibilityPlaceholder();
      html.document.body.append(_placeholder);

      html.Event event = dispatchMouseEvent('mousedown', target: _placeholder);
      bool shouldForwardToFramework =
          desktopEnableSemantics.tryEnableSemantics(event);

      expect(shouldForwardToFramework, false);
    });

    test(
        'After max number of relevant events, events should be forwarded to the framework',
        () async {
      // Prework. Attach the placeholder to dom.
      _placeholder = desktopEnableSemantics.prepareAccesibilityPlaceholder();
      html.document.body.append(_placeholder);

      html.Event event = dispatchMouseEvent('mousedown', target: _placeholder);
      bool shouldForwardToFramework =
          desktopEnableSemantics.tryEnableSemantics(event);

      expect(shouldForwardToFramework, false);

      // Send max number of events;
      for (int i = 1; i <= kMaxSemanticsActivationAttempts; i++) {
        event = dispatchPointerEvent('mousedown', target: _placeholder);
        shouldForwardToFramework =
            desktopEnableSemantics.tryEnableSemantics(event);
      }

      expect(shouldForwardToFramework, true);
    });
  });

  group('$MobileEnableSemantics', () {
    MobileEnableSemantics mobileEnableSemantics;
    html.Element _placeholder;

    setUp(() {
      mobileEnableSemantics = MobileEnableSemantics();
    });

    tearDown(() {
      if (_placeholder != null) {
        _placeholder.remove();
      }
    });

    test('prepare accesibility placeholder', () async {
      _placeholder = mobileEnableSemantics.prepareAccesibilityPlaceholder();

      expect(_placeholder.getAttribute('role'), 'button');

      html.document.body.append(_placeholder);

      // Placeholder should cover all the screen on a mobile device.
      final num bodyHeight = html.window.innerHeight;
      final num bodyWidht = html.window.innerWidth;

      expect(_placeholder.getBoundingClientRect().height, bodyHeight);
      expect(_placeholder.getBoundingClientRect().width, bodyWidht);
    });

    test('Not relevant events should be forwarded to the framework', () async {
      final html.Event event = dispatchTouchEvent('touchcancel');
      bool shouldForwardToFramework =
          mobileEnableSemantics.tryEnableSemantics(event);

      expect(shouldForwardToFramework, true);
    });
  });
}

html.PointerEvent dispatchPointerEvent(String type, {html.EventTarget target}) {
  target ??= html.window;

  final Function jsKeyboardEvent =
      js_util.getProperty(html.window, 'PointerEvent');
  final List<dynamic> eventArgs = <dynamic>[
    type,
  ];
  final html.PointerEvent event =
      js_util.callConstructor(jsKeyboardEvent, js_util.jsify(eventArgs));

  target.dispatchEvent(event);

  return event;
}

html.MouseEvent dispatchMouseEvent(String type, {html.EventTarget target}) {
  target ??= html.window;

  final Function jsKeyboardEvent =
      js_util.getProperty(html.window, 'MouseEvent');
  final List<dynamic> eventArgs = <dynamic>[
    type,
  ];
  final html.MouseEvent event =
      js_util.callConstructor(jsKeyboardEvent, js_util.jsify(eventArgs));

  target.dispatchEvent(event);

  return event;
}

html.TouchEvent dispatchTouchEvent(String type, {html.EventTarget target}) {
  target ??= html.window;

  final Function jsKeyboardEvent =
      js_util.getProperty(html.window, 'TouchEvent');
  final List<dynamic> eventArgs = <dynamic>[
    type,
  ];
  final html.TouchEvent event =
      js_util.callConstructor(jsKeyboardEvent, js_util.jsify(eventArgs));

  target.dispatchEvent(event);

  return event;
}
