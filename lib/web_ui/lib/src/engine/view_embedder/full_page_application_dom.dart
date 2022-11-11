// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js.dart';

import '../dom.dart';
import '../util.dart' show assertionsEnabled;
import 'application_dom.dart';


class FullPageApplicationDom extends ApplicationDom {

  /// Configures the screen, such as scaling.
  ///
  /// Created in [applyViewportMeta].
  late DomHTMLMetaElement? _viewportMeta;

  @override
  void applyViewportMeta() {
    for (final DomElement viewportMeta
        in domDocument.head!.querySelectorAll('meta[name="viewport"]')) {
      if (assertionsEnabled) {
        // Filter out the meta tag that the engine placed on the page. This is
        // to avoid UI flicker during hot restart. Hot restart will clean up the
        // old meta tag synchronously with the first post-restart frame.
        if (!viewportMeta.hasAttribute('flt-viewport')) {
          print(
            'WARNING: found an existing <meta name="viewport"> tag. Flutter '
            'Web uses its own viewport configuration for better compatibility '
            'with Flutter. This tag will be replaced.',
          );
        }
      }
      viewportMeta.remove();
    }

    // The meta viewport is always removed by the for method above, so we don't
    // need to do anything else here, other than create it again.
    _viewportMeta = createDomHTMLMetaElement()
      ..setAttribute('flt-viewport', '')
      ..name = 'viewport'
      ..content = 'width=device-width, initial-scale=1.0, '
          'maximum-scale=1.0, user-scalable=no';

    domDocument.head!.append(_viewportMeta!);

    registerElementForCleanup(_viewportMeta!);
  }

  void createGlassPane() {}
  void createSceneHost() {}
  void createSemanticsHost() {}
  void prepareAccessibilityPlaceholder() {}
  void assembleGlassPane() {}

  void addScene(DomElement? sceneElement) {}

  @override
  void setMetricsChangeHandler(void Function(DomEvent? event) handler) {
    late DomSubscription subscription;

    if (domWindow.visualViewport != null) {
      subscription = DomSubscription(domWindow.visualViewport!, 'resize',
          allowInterop(handler));
    } else {
      subscription = DomSubscription(domWindow, 'resize',
          allowInterop(handler));
    }

    registerSubscriptionForCleanup(subscription);
  }

  @override
  void setLanguageChangeHandler(void Function(DomEvent event) handler) {
    final DomSubscription subscription = DomSubscription(domWindow,
          'languagechange', allowInterop(handler));

    registerSubscriptionForCleanup(subscription);
  }

}
