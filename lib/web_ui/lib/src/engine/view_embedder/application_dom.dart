// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart';

import '../dom.dart';

import 'custom_element_application_dom.dart';
import 'full_page_application_dom.dart';
import 'hot_restart_cache_handler.dart';

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

  /// Returns the 'type' (a String for debugging) of the ApplicationDom instance.
  String get type;

  /// Sets-up the viewport Meta-Tag for the app.
  void applyViewportMeta();

  /// Sets the global styles for the hostElement of this Flutter web app.
  ///
  /// [font] is the CSS shorthand property to set all the different font properties.
  void setHostStyles({
    required String font,
  });

  /// Sets an attribute in the hostElement.
  ///
  /// Like "flt-renderer" or "flt-build-mode".
  void setHostAttribute(String name, String value);

  /// Attaches the glassPane element into the hostElement.
  void attachGlassPane(DomElement glassPaneElement);

  /// Attaches the resourceHost element into the hostElement.
  void attachResourcesHost(DomElement resourceHost, { DomElement? nextTo });

  /// Register a listener for window resize events
  void setMetricsChangeHandler(void Function(DomEvent? event) handler);

  /// Register a listener for locale change events.
  void setLanguageChangeHandler(void Function(DomEvent event) handler);

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
