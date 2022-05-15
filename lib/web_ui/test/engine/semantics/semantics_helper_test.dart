// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine/browser_detection.dart';
import 'package:ui/src/engine/dom.dart';
import 'package:ui/src/engine/pointer_binding.dart';
import 'package:ui/src/engine/semantics.dart';

const PointerSupportDetector _defaultSupportDetector = PointerSupportDetector();

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$DesktopSemanticsEnabler', () {
    late DesktopSemanticsEnabler desktopSemanticsEnabler;
    late DomElement? _placeholder;

    setUp(() {
      EngineSemanticsOwner.instance.semanticsEnabled = false;
      desktopSemanticsEnabler = DesktopSemanticsEnabler();
      _placeholder = desktopSemanticsEnabler.prepareAccessibilityPlaceholder();
      domDocument.body!.append(_placeholder!);
    });

    tearDown(() {
      expect(_placeholder, isNotNull,
          reason: 'Expected the test to create a placeholder');
      _placeholder!.remove();
      EngineSemanticsOwner.instance.semanticsEnabled = false;
    });

    test('prepare accessibility placeholder', () async {
      expect(_placeholder!.getAttribute('role'), 'button');
      expect(_placeholder!.getAttribute('aria-live'), 'polite');
      expect(_placeholder!.getAttribute('tabindex'), '0');

      domDocument.body!.append(_placeholder!);

      expect(domDocument.getElementsByTagName('flt-semantics-placeholder'),
          isNotEmpty);

      expect(_placeholder!.getBoundingClientRect().height, 1);
      expect(_placeholder!.getBoundingClientRect().width, 1);
      expect(_placeholder!.getBoundingClientRect().top, -1);
      expect(_placeholder!.getBoundingClientRect().left, -1);
    });

    test('Not relevant events should be forwarded to the framework', () async {
      // Attach the placeholder to dom.
      domDocument.body!.append(_placeholder!);

      DomEvent event = createDomEvent('Event', 'mousemove');
      bool shouldForwardToFramework =
          desktopSemanticsEnabler.tryEnableSemantics(event);

      expect(shouldForwardToFramework, isTrue);

      // Pointer events are not defined in webkit.
      if (browserEngine != BrowserEngine.webkit) {
        event = createDomEvent('Event', 'pointermove');
        shouldForwardToFramework =
            desktopSemanticsEnabler.tryEnableSemantics(event);

        expect(shouldForwardToFramework, isTrue);
      }
    });

    test(
        'Relevant events targeting placeholder should not be forwarded to the framework',
        () async {
      final DomEvent event = createDomEvent('Event', 'mousedown');
      _placeholder!.dispatchEvent(event);

      final bool shouldForwardToFramework =
          desktopSemanticsEnabler.tryEnableSemantics(event);

      expect(shouldForwardToFramework, isFalse);
    });

    test('disposes of the placeholder', () {
      domDocument.body!.append(_placeholder!);

      expect(_placeholder!.isConnected, isTrue);
      desktopSemanticsEnabler.dispose();
      expect(_placeholder!.isConnected, isFalse);
    });
  }, skip: isMobile);

  group(
    '$MobileSemanticsEnabler',
    () {
      late MobileSemanticsEnabler mobileSemanticsEnabler;
      DomElement? _placeholder;

      setUp(() {
        EngineSemanticsOwner.instance.semanticsEnabled = false;
        mobileSemanticsEnabler = MobileSemanticsEnabler();
        _placeholder = mobileSemanticsEnabler.prepareAccessibilityPlaceholder();
        domDocument.body!.append(_placeholder!);
      });

      tearDown(() {
        _placeholder!.remove();
        EngineSemanticsOwner.instance.semanticsEnabled = false;
      });

      test('prepare accessibility placeholder', () async {
        expect(_placeholder!.getAttribute('role'), 'button');

        // Placeholder should cover all the screen on a mobile device.
        final num bodyHeight = domWindow.innerHeight!;
        final num bodyWidth = domWindow.innerWidth!;

        expect(_placeholder!.getBoundingClientRect().height, bodyHeight);
        expect(_placeholder!.getBoundingClientRect().width, bodyWidth);
      });

      test('Non-relevant events should be forwarded to the framework',
          () async {
        DomEvent event;
        if (_defaultSupportDetector.hasPointerEvents) {
          event = createDomPointerEvent('pointermove');
        } else if (_defaultSupportDetector.hasTouchEvents) {
          event = createDomTouchEvent('touchcancel');
        } else {
          event = createDomMouseEvent('mousemove');
        }

        final bool shouldForwardToFramework =
            mobileSemanticsEnabler.tryEnableSemantics(event);

        expect(shouldForwardToFramework, isTrue);
      });

      test('Enables semantics when receiving a relevant event', () {
        expect(mobileSemanticsEnabler.semanticsActivationTimer, isNull);

        // Send a click off center
        _placeholder!.dispatchEvent(createDomMouseEvent(
          'click',
          <Object?, Object?>{ 'clientX': 0,
            'clientY': 0 }
        ));
        expect(mobileSemanticsEnabler.semanticsActivationTimer, isNull);

        // Send a click at center
        final DomRect activatingElementRect =
            _placeholder!.getBoundingClientRect();
        final int midX = (activatingElementRect.left +
                (activatingElementRect.right - activatingElementRect.left) / 2)
            .toInt();
        final int midY = (activatingElementRect.top +
                (activatingElementRect.bottom - activatingElementRect.top) / 2)
            .toInt();
        _placeholder!.dispatchEvent(createDomMouseEvent(
          'click',
          <Object?, Object?>{ 'clientX': midX,
            'clientY': midY }
        ));
        expect(mobileSemanticsEnabler.semanticsActivationTimer, isNotNull);
      });
    },
    // We can run `MobileSemanticsEnabler` tests in mobile browsers and in desktop Chrome.
    skip: isDesktop && browserEngine != BrowserEngine.blink,
  );
}
