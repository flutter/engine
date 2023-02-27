// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
@JS()
library embedder_test; // We need this to mess with the ShadowDOM.

import 'package:js/js.dart';
import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  test('populates flt-renderer and flt-build-mode', () {
    FlutterViewEmbedder();
    expect(domDocument.body!.getAttribute('flt-renderer'),
        'html (requested explicitly)');
    expect(domDocument.body!.getAttribute('flt-build-mode'), 'debug');
  });

  test('innerHeight/innerWidth are equal to visualViewport height and width',
      () {
    if (domWindow.visualViewport != null) {
      expect(domWindow.visualViewport!.width, domWindow.innerWidth);
      expect(domWindow.visualViewport!.height, domWindow.innerHeight);
    }
  });

  test('replaces viewport meta tags during style reset', () {
    final DomHTMLMetaElement existingMeta = createDomHTMLMetaElement()
      ..name = 'viewport'
      ..content = 'foo=bar';
    domDocument.head!.append(existingMeta);
    expect(existingMeta.isConnected, isTrue);

    final FlutterViewEmbedder embedder = FlutterViewEmbedder();
    embedder.reset();
  },
      // TODO(ferhat): https://github.com/flutter/flutter/issues/46638
      skip: browserEngine == BrowserEngine.firefox);

  test('accesibility placeholder is attached after creation', () {
    final FlutterViewEmbedder embedder = FlutterViewEmbedder();

    expect(
      embedder.glassPaneShadow.querySelectorAll('flt-semantics-placeholder'),
      isNotEmpty,
    );
  });

  test('should add/remove global resource', () {
    final FlutterViewEmbedder embedder = FlutterViewEmbedder();
    final DomHTMLDivElement resource = createDomHTMLDivElement();
    embedder.addResource(resource);
    final DomElement? resourceRoot = resource.parent;
    expect(resourceRoot, isNotNull);
    expect(resourceRoot!.childNodes.length, 1);
    embedder.removeResource(resource);
    expect(resourceRoot.childNodes.length, 0);
  });

  test('hide placeholder text for textfield', () {
    final FlutterViewEmbedder embedder = FlutterViewEmbedder();
    final DomHTMLInputElement regularTextField = createDomHTMLInputElement();
    regularTextField.placeholder = 'Now you see me';
    embedder.addResource(regularTextField);

    regularTextField.focus();
    DomCSSStyleDeclaration? style = domWindow.getComputedStyle(
        embedder.glassPaneShadow.querySelector('input')!,
        '::placeholder');
    expect(style, isNotNull);
    expect(style.opacity, isNot('0'));

    final DomHTMLInputElement textField = createDomHTMLInputElement();
    textField.placeholder = 'Now you dont';
    textField.classList.add('flt-text-editing');
    embedder.addResource(textField);

    textField.focus();
    style = domWindow.getComputedStyle(
        embedder.glassPaneShadow.querySelector('input.flt-text-editing')!,
        '::placeholder');
    expect(style, isNotNull);
    expect(style.opacity, '0');
  }, skip: browserEngine != BrowserEngine.firefox);

  group('Shadow root and styles', () {
    final FlutterViewEmbedder embedder = FlutterViewEmbedder();

    test('throws when shadowDom is not available', () {
      final dynamic oldAttachShadow = attachShadow;
      expect(oldAttachShadow, isNotNull);

      attachShadow = null; // Break ShadowDOM

      expect(() => FlutterViewEmbedder(), throwsUnsupportedError);
      attachShadow = oldAttachShadow; // Restore ShadowDOM
    });

    test('Initializes and attaches a shadow root', () {
      expect(domInstanceOfString(embedder.glassPaneShadow, 'ShadowRoot'), isTrue);
      expect(embedder.glassPaneShadow.host, embedder.glassPaneElement);
      expect(embedder.glassPaneShadow, embedder.glassPaneElement.shadowRoot);

      // The shadow root should be initialized with correct parameters.
      expect(embedder.glassPaneShadow.mode, 'open');
      if (browserEngine != BrowserEngine.firefox &&
          browserEngine != BrowserEngine.webkit) {
        // Older versions of Safari and Firefox don't support this flag yet.
        // See: https://caniuse.com/mdn-api_shadowroot_delegatesfocus
        expect(embedder.glassPaneShadow.delegatesFocus, isFalse);
      }
    });

    test('Attaches a stylesheet to the shadow root', () {
      final DomElement? style =
          embedder.glassPaneShadow.querySelector('#flt-internals-stylesheet');

      expect(style, isNotNull);
      expect(style!.tagName, equalsIgnoringCase('style'));
    });

    test('(Self-test) hasCssRule can extract rules', () {
      final DomElement? style =
          embedder.glassPaneShadow.querySelector('#flt-internals-stylesheet');

      final bool hasRule = hasCssRule(style,
          selector: '.flt-text-editing::placeholder',
          declaration: 'opacity: 0');

      final bool hasFakeRule = hasCssRule(style,
          selector: 'input::selection', declaration: 'color: #fabada;');

      expect(hasRule, isTrue);
      expect(hasFakeRule, isFalse);
    });

    test('Attaches outrageous text styles to flt-scene-host', () {
      final DomElement? style =
          embedder.glassPaneShadow.querySelector('#flt-internals-stylesheet');

      final bool hasColorRed = hasCssRule(style,
          selector: 'flt-scene-host', declaration: 'color: red');

      bool hasFont = false;
      if (isSafari) {
        // Safari expands the shorthand rules, so we check for all we've set (separately).
        hasFont = hasCssRule(style,
                selector: 'flt-scene-host',
                declaration: 'font-family: monospace') &&
            hasCssRule(style,
                selector: 'flt-scene-host', declaration: 'font-size: 14px');
      } else {
        hasFont = hasCssRule(style,
            selector: 'flt-scene-host', declaration: 'font: 14px monospace');
      }

      expect(hasColorRed, isTrue,
          reason: 'Should make foreground color red within scene host.');
      expect(hasFont, isTrue, reason: 'Should pass default css font.');
    });

    test('Attaches styling to remove password reveal icons on Edge', () {
      final DomElement? style =
          embedder.glassPaneShadow.querySelector('#flt-internals-stylesheet');

      // Check that style.sheet! contains input::-ms-reveal rule
      final bool hidesRevealIcons = hasCssRule(style,
          selector: 'input::-ms-reveal', declaration: 'display: none');

      final bool codeRanInFakeyBrowser = hasCssRule(style,
          selector: 'input.fallback-for-fakey-browser-in-ci',
          declaration: 'display: none');

      if (codeRanInFakeyBrowser) {
        print('Please, fix https://github.com/flutter/flutter/issues/116302');
      }

      expect(hidesRevealIcons || codeRanInFakeyBrowser, isTrue,
          reason: 'In Edge, stylesheet must contain "input::-ms-reveal" rule.');
    }, skip: !isEdge);

    test('Does not attach the Edge-specific style tag on non-Edge browsers',
        () {
      final DomElement? style =
          embedder.glassPaneShadow.querySelector('#flt-internals-stylesheet');

      // Check that style.sheet! contains input::-ms-reveal rule
      final bool hidesRevealIcons = hasCssRule(style,
          selector: 'input::-ms-reveal', declaration: 'display: none');

      expect(hidesRevealIcons, isFalse);
    }, skip: isEdge);

    test(
        'Attaches styles to hide the autofill overlay for browsers that support it',
        () {
      final DomElement? style =
          embedder.glassPaneShadow.querySelector('#flt-internals-stylesheet');
      final String vendorPrefix = (isSafari || isFirefox) ? '' : '-webkit-';
      final bool autofillOverlay = hasCssRule(style,
          selector: '.transparentTextEditing:${vendorPrefix}autofill',
          declaration: 'opacity: 0 !important');
      final bool autofillOverlayHovered = hasCssRule(style,
          selector: '.transparentTextEditing:${vendorPrefix}autofill:hover',
          declaration: 'opacity: 0 !important');
      final bool autofillOverlayFocused = hasCssRule(style,
          selector: '.transparentTextEditing:${vendorPrefix}autofill:focus',
          declaration: 'opacity: 0 !important');
      final bool autofillOverlayActive = hasCssRule(style,
          selector: '.transparentTextEditing:${vendorPrefix}autofill:active',
          declaration: 'opacity: 0 !important');

      expect(autofillOverlay, isTrue);
      expect(autofillOverlayHovered, isTrue);
      expect(autofillOverlayFocused, isTrue);
      expect(autofillOverlayActive, isTrue);
    }, skip: !browserHasAutofillOverlay());
  });
}

@JS('Element.prototype.attachShadow')
external dynamic get attachShadow;

@JS('Element.prototype.attachShadow')
external set attachShadow(dynamic x);

/// Finds out whether a given CSS Rule ([selector] { [declaration]; }) exists in a [styleSheet].
bool hasCssRule(
  DomElement? styleSheet, {
  required String selector,
  required String declaration,
}) {
  assert(styleSheet != null);
  assert((styleSheet! as DomHTMLStyleElement).sheet != null);

  // regexr.com/740ff
  final RegExp ruleLike =
      RegExp('[^{]*(?:$selector)[^{]*{[^}]*(?:$declaration)[^}]*}');

  final DomCSSStyleSheet sheet =
      (styleSheet! as DomHTMLStyleElement).sheet! as DomCSSStyleSheet;

  // Check that the cssText of any rule matches the ruleLike RegExp.
  return sheet.cssRules
      .map((DomCSSRule rule) => rule.cssText)
      .any((String rule) => ruleLike.hasMatch(rule));
}
