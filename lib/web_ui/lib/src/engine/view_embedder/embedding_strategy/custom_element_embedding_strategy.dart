// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine/dom.dart';

import 'embedding_strategy.dart';

class CustomElementEmbeddingStrategy extends EmbeddingStrategy {
  CustomElementEmbeddingStrategy(this._hostElement) {
    // Clear children...
    while (_hostElement.firstChild != null) {
      _hostElement.removeChild(_hostElement.lastChild!);
    }
  }

  final DomElement _hostElement;

  @override
  void initialize({
    Map<String, String>? embedderMetadata,
  }) {
    // ignore:avoid_function_literals_in_foreach_calls
    embedderMetadata?.entries.forEach((MapEntry<String, String> entry) {
      _setHostAttribute(entry.key, entry.value);
    });
    _setHostAttribute('flt-embedding', 'custom-element');
  }

  @override
  void attachGlassPane(DomElement glassPaneElement) {
    glassPaneElement
      ..style.width = '100%'
      ..style.height = '100%'
      ..style.display = 'block'
      ..style.overflow = 'hidden'
      ..style.position = 'relative';

    _hostElement.appendChild(glassPaneElement);

    registerElementForCleanup(glassPaneElement);
  }

  @override
  void attachResourcesHost(DomElement resourceHost, {DomElement? nextTo}) {
    _hostElement.insertBefore(resourceHost, nextTo);

    registerElementForCleanup(resourceHost);
  }

  void _setHostAttribute(String name, String value) {
    _hostElement.setAttribute(name, value);
  }
}
