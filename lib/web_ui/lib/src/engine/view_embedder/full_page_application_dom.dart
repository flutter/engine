// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js.dart';

import '../dom.dart';
import '../util.dart' show assertionsEnabled, setElementStyle;
import 'application_dom.dart';


class FullPageApplicationDom extends ApplicationDom {
  @override
  final String type = 'full-page';

  @override
  void setHostAttribute(String name, String value) {
    domDocument.body!.setAttribute(name, value);
  }

  @override
  void setHostStyles({
    required String font,
  }) {
    final DomHTMLBodyElement bodyElement = domDocument.body!;

    setElementStyle(bodyElement, 'position', 'fixed');
    setElementStyle(bodyElement, 'top', '0');
    setElementStyle(bodyElement, 'right', '0');
    setElementStyle(bodyElement, 'bottom', '0');
    setElementStyle(bodyElement, 'left', '0');
    setElementStyle(bodyElement, 'overflow', 'hidden');
    setElementStyle(bodyElement, 'padding', '0');
    setElementStyle(bodyElement, 'margin', '0');

    // TODO(yjbanov): fix this when KVM I/O support is added. Currently scroll
    //                using drag, and text selection interferes.
    setElementStyle(bodyElement, 'user-select', 'none');
    setElementStyle(bodyElement, '-webkit-user-select', 'none');

    // This is required to prevent the browser from doing any native touch
    // handling. If this is not done, the browser doesn't report 'pointermove'
    // events properly.
    setElementStyle(bodyElement, 'touch-action', 'none');

    // These are intentionally outrageous font parameters to make sure that the
    // apps fully specify their text styles.
    setElementStyle(bodyElement, 'font', font);
    setElementStyle(bodyElement, 'color', 'red');
  }

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
    final DomHTMLMetaElement viewportMeta = createDomHTMLMetaElement()
      ..setAttribute('flt-viewport', '')
      ..name = 'viewport'
      ..content = 'width=device-width, initial-scale=1.0, '
          'maximum-scale=1.0, user-scalable=no';

    domDocument.head!.append(viewportMeta);

    registerElementForCleanup(viewportMeta);
  }

  @override
  void attachGlassPane(DomElement glassPaneElement) {
    /// Tweaks style so the glassPane works well with the hostElement.
    glassPaneElement.style
      ..position = 'absolute'
      ..top = '0'
      ..right = '0'
      ..bottom = '0'
      ..left = '0';

    domDocument.body!.append(glassPaneElement);

    registerElementForCleanup(glassPaneElement);
  }

  @override
  void attachResourcesHost(DomElement resourceHost, { DomElement? nextTo }) {
    domDocument.body!.insertBefore(resourceHost, nextTo);

    registerElementForCleanup(resourceHost);
  }

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
