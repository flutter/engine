// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
@JS()
library dom_renderer_test; // We need this to mess with the ShadowDOM.

import 'dart:html' as html;

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
    expect(html.document.body!.attributes['flt-renderer'],
        'html (requested explicitly)');
    expect(html.document.body!.attributes['flt-build-mode'], 'debug');
  });

  test('can set style properties on elements', () {
    final html.Element element = html.document.createElement('div');
    setElementStyle(element, 'color', 'red');
    expect(element.style.color, 'red');
  });

  test('can remove style properties from elements', () {
    final html.Element element = html.document.createElement('div');
    setElementStyle(element, 'color', 'blue');
    expect(element.style.color, 'blue');
    setElementStyle(element, 'color', null);
    expect(element.style.color, '');
  });

  test('innerHeight/innerWidth are equal to visualViewport height and width',
      () {
    if (html.window.visualViewport != null) {
      expect(html.window.visualViewport!.width, html.window.innerWidth);
      expect(html.window.visualViewport!.height, html.window.innerHeight);
    }
  });

  test('replaces viewport meta tags during style reset', () {
    final html.MetaElement existingMeta = html.MetaElement()
      ..name = 'viewport'
      ..content = 'foo=bar';
    html.document.head!.append(existingMeta);
    expect(existingMeta.isConnected, isTrue);

    final FlutterViewEmbedder renderer = FlutterViewEmbedder();
    renderer.reset();
  },
      // TODO(ferhat): https://github.com/flutter/flutter/issues/46638
      // TODO(ferhat): https://github.com/flutter/flutter/issues/50828
      skip: browserEngine == BrowserEngine.firefox ||
          browserEngine == BrowserEngine.edge);

  test('accesibility placeholder is attached after creation', () {
    final FlutterViewEmbedder renderer = FlutterViewEmbedder();

    expect(
      renderer.glassPaneShadow?.querySelectorAll('flt-semantics-placeholder'),
      isNotEmpty,
    );
  });

  test('renders a shadowRoot by default', () {
    final FlutterViewEmbedder renderer = FlutterViewEmbedder();

    final HostNode hostNode = renderer.glassPaneShadow!;

    expect(hostNode.node, isA<html.ShadowRoot>());
  });

  test('starts without shadowDom available too', () {
    final dynamic oldAttachShadow = attachShadow;
    expect(oldAttachShadow, isNotNull);

    attachShadow = null; // Break ShadowDOM

    final FlutterViewEmbedder renderer = FlutterViewEmbedder();

    final HostNode hostNode = renderer.glassPaneShadow!;

    expect(hostNode.node, isA<html.Element>());
    expect(
      (hostNode.node as html.Element).tagName,
      equalsIgnoringCase('flt-element-host-node'),
    );

    attachShadow = oldAttachShadow; // Restore ShadowDOM
  });

  test('should add/remove global resource', () {
    final FlutterViewEmbedder renderer = FlutterViewEmbedder();
    final html.DivElement resource = html.DivElement();
    renderer.addResource(resource);
    final html.Element? resourceRoot = resource.parent;
    expect(resourceRoot, isNotNull);
    expect(resourceRoot!.childNodes.length, 1);
    renderer.removeResource(resource);
    expect(resourceRoot.childNodes.length, 0);
  });

  test('hide placeholder text for textfield', () {
    final FlutterViewEmbedder renderer = FlutterViewEmbedder();
    final html.InputElement regularTextField = html.InputElement();
    regularTextField.placeholder = 'Now you see me';
    renderer.addResource(regularTextField);

    regularTextField.focus();
    html.CssStyleDeclaration? style = renderer.glassPaneShadow?.querySelector('input')?.getComputedStyle('::placeholder');
    expect(style, isNotNull);
    expect(style?.opacity, isNot('0'));

    final html.InputElement textField = html.InputElement();
    textField.placeholder = 'Now you dont';
    textField.classes.add('flt-text-editing');
    renderer.addResource(textField);

    textField.focus();
    style = renderer.glassPaneShadow?.querySelector('input.flt-text-editing')?.getComputedStyle('::placeholder');
    expect(style, isNotNull);
    expect(style?.opacity, '0');
  }, skip: browserEngine != BrowserEngine.firefox);
}

@JS('Element.prototype.attachShadow')
external dynamic get attachShadow;

@JS('Element.prototype.attachShadow')
external set attachShadow(dynamic x);
