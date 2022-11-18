// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import '../dom.dart';
import 'application_dom.dart';

class CustomElementApplicationDom extends ApplicationDom {

  CustomElementApplicationDom(this._hostElement) {
    // Clear children...
    while(_hostElement.firstChild != null) {
      _hostElement.removeChild(_hostElement.lastChild!);
    }

    // Hook up a resize observer on the hostElement (if supported!).
    //
    // Should all this code live in the DimensionsProvider classes?
    _resizeObserver = createDomResizeObserver(
      (List<DomResizeObserverEntry> entries, DomResizeObserver _) {
        entries.forEach(_streamController.add);
      }
    );

    assert(() {
      if (_resizeObserver == null) {
        domWindow.console.warn('ResizeObserver API not supported. Flutter will not resize with its hostElement.');
      }
      return true;
    }());

    _resizeObserver?.observe(_hostElement);
  }

  final DomElement _hostElement;
  late DomResizeObserver? _resizeObserver;

  final StreamController<DomResizeObserverEntry> _streamController =
    StreamController<DomResizeObserverEntry>.broadcast();

  @override
  final String type = 'custom-element';

  @override
  void applyViewportMeta() {
    // NOOP
  }

  @override
  void setHostStyles({
    required String font,
  }) {
    _hostElement
      ..style.position = 'relative'
      ..style.overflow = 'hidden';
  }

  @override
  void setHostAttribute(String name, String value) {
    _hostElement.setAttribute(name, value);
  }

  @override
  void attachGlassPane(DomElement glassPaneElement) {
    glassPaneElement
      ..style.width = '100%'
      ..style.height = '100%'
      ..style.display = 'block';

    _hostElement.appendChild(glassPaneElement);

    registerElementForCleanup(glassPaneElement);
  }

  @override
  void attachResourcesHost(DomElement resourceHost, {DomElement? nextTo }) {
    _hostElement.insertBefore(resourceHost, nextTo);

    registerElementForCleanup(resourceHost);
  }

  @override
  void setMetricsChangeHandler(void Function(DomEvent? event) handler) {
    _streamController.stream.listen((DomResizeObserverEntry _) {
      handler(null);
    });
  }

  @override
  void setLanguageChangeHandler(void Function(DomEvent event) handler) {
    // How do we detect the language changes? Is this global? Should we look
    // at the lang= attribute of the hostElement?
  }

  /// This should "clean" up anything handled by the [ApplicationDom] instance.
  @override
  void onHotRestart() {
    _resizeObserver?.disconnect();
    _streamController.close();
    super.onHotRestart();
  }
}
