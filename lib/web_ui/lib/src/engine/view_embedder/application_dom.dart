// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:meta/meta.dart';
import 'package:ui/src/engine/util.dart';
import 'package:ui/ui.dart' as ui;

import '../dom.dart';

import '../safe_browser_api.dart';
import 'custom_element_application_dom.dart';
import 'full_page_application_dom.dart';

/// Provides the API that the FlutterViewEmbedder uses to interact with the DOM.
///
/// The base class handles "global" stuff that is shared across implementations,
/// like handling hot-restart cleanup.
///
/// This class is specialized to handle different types of DOM embeddings:
///
/// * [FullPageApplicationDom] - The default behavior, where flutter takes
///   control of the whole web page. This is how Flutter Web used to operate.
/// * [CustomElementApplicationDom] - Flutter is rendered inside a custom host
///   element, provided by the web app programmer through the engine
///   initialization.
abstract class ApplicationDom {
  @mustCallSuper
  ApplicationDom() {
    // Prepare some global stuff...
    assert(() {
      _hotRestartCache = HotRestartCacheHandler();
      return true;
    }());
  }

  factory ApplicationDom.create({DomElement? hostElement}) {
    if (hostElement != null) {
      return CustomElementApplicationDom(hostElement);
    } else {
      return FullPageApplicationDom();
    }
  }

  /// Keeps a list of elements to be cleaned up at hot-restart.
  HotRestartCacheHandler? _hotRestartCache;

  /// Sets-up the viewport Meta-Tag for the app.
  void applyViewportMeta() {}

  void createGlassPane() {}
  void createSceneHost() {}
  void createSemanticsHost() {}
  void prepareAccessibilityPlaceholder() {}
  void assembleGlassPane() {}

  void addScene(DomElement? sceneElement) {}

  /// Register a listener for window resize events
  void setMetricsChangeHandler(void Function(DomEvent? event) handler) {}

  /// Register a listener for locale change events.
  void setLanguageChangeHandler(void Function(DomEvent event) handler) {}

  /// A callback that runs when hot restart is triggered.
  ///
  /// This should "clean" up anything handled by the [ApplicationDom] instance.
  @mustCallSuper
  void onHotRestart() {
    _hotRestartCache?.clearAllSubscriptions();
  }

  /// Registers a [DomSubscription] to be cleaned up [onHotRestart].
  @mustCallSuper
  void registerSubscriptionForCleanup(DomSubscription subscription) {
    _hotRestartCache?.registerSubscription(subscription);
  }

  /// Registers a [DomElement] to be cleaned up after hot restart.
  @mustCallSuper
  void registerElementForCleanup(DomElement element) {
    _hotRestartCache?.registerElement(element);
  }
}

/// Handles elements and subscriptions that need to be cleared on hot-restart.
class HotRestartCacheHandler {
  HotRestartCacheHandler([this.storeName = '__flutter_state']) {
    if (_elements.isNotEmpty) {
      // We are in a post hot-restart world, clear the elements now.
      clearAllElements();
    }
  }

  /// This is state persistent across hot restarts that indicates what
  /// to clear.  Delay removal of old visible state to make the
  /// transition appear smooth.
  final String storeName;

  /// The js-interop layer backing [_elements].
  ///
  /// They're stored in a js global with name [storeName], and removed from the
  /// DOM when the app repaints...
  late List<DomElement?>? _jsElements;

  /// The elements that need to be cleaned up after hot-restart.
  List<DomElement?> get _elements {
    _jsElements = getJsProperty<List<DomElement?>?>(domWindow, storeName);
    if (_jsElements == null) {
      _jsElements = <DomElement?>[];
      setJsProperty(domWindow, storeName, _jsElements);
    }
    return _jsElements!;
  }

  /// The subscriptions that need to be cleaned up on hot-restart.
  final List<DomSubscription> _subscriptions = <DomSubscription>[];

  void registerSubscription(DomSubscription subscription) {
    _subscriptions.add(subscription);
  }

  void registerElement(DomElement element) {
    _elements.add(element);
  }

  void clearAllSubscriptions() {
    print('Clearing subscriptions');
    print(_subscriptions);
    for (final DomSubscription subscription in _subscriptions) {
      subscription.cancel();
    }
    _subscriptions.clear();
  }

  void clearAllElements() {
    print('Clearing elements');
    print(_elements);
    for (final DomElement? element in _elements) {
      element?.remove();
    }
    _elements.clear();
  }
}
