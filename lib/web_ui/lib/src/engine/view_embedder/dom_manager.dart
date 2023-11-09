// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../embedder.dart';
import '../global_styles.dart';

/// Manages DOM elements and the DOM structure for a [ui.FlutterView].
///
/// Here's the general DOM structure of a Flutter View:
///
/// [rootElement] <flutter-view>
///   |
///   +- [platformViewsHost] <flt-glass-pane>
///   |    |
///   |    +- [renderingHost] #shadow-root
///   |    |    |
///   |    |    +- <flt-semantics-placeholder>
///   |    |    |
///   |    |    +- <flt-scene-host>
///   |    |    |    |
///   |    |    |    +- <flt-scene>
///   |    |    |
///   |    |    +- <flt-announcement-host>
///   |    |
///   |    +- ...platform views
///   |
///   +- [textEditingHost] <text-editing-host>
///   |    |
///   |    +- ...text fields
///   |
///   +- [semanticsHost] <semantics-host>
///        |
///        +- ...semantics nodes
///
class DomManager {
  DomManager.fromFlutterViewEmbedderDEPRECATED(this._embedder);

  /// The tag name for the Flutter View root element.
  static const String flutterViewTagName = 'flutter-view';

  /// The tag name for the glass-pane.
  static const String glassPaneTagName = 'flt-glass-pane';

  /// The tag name for the scene host.
  static const String sceneHostTagName = 'flt-scene-host';

  /// The tag name for the semantics host.
  static const String semanticsHostTagName = 'flt-semantics-host';

  /// The tag name for the accessibility announcements host.
  static const String announcementsHostTagName = 'flt-announcement-host';

  final FlutterViewEmbedder _embedder;

  /// The root DOM element for the entire Flutter View.
  ///
  /// This is where input events are captured, such as pointer events.
  ///
  /// If semantics is enabled, this element also contains the semantics DOM tree,
  /// which captures semantics input events.
  DomElement get rootElement => _embedder.flutterViewElementDEPRECATED;

  /// Hosts all platform view elements.
  DomElement get platformViewsHost => _embedder.glassPaneElementDEPRECATED;

  /// Hosts all rendering elements and canvases.
  DomShadowRoot get renderingHost => _embedder.glassPaneShadowDEPRECATED;

  /// Hosts all text editing elements.
  DomElement get textEditingHost => _embedder.textEditingHostNodeDEPRECATED;

  /// Hosts the semantics tree.
  ///
  /// This element is in front of the [renderingHost] and [platformViewsHost].
  /// Otherwise, the phone will disable focusing by touch, only by tabbing
  /// around the UI.
  DomElement get semanticsHost => _embedder.semanticsHostElementDEPRECATED;
}

/// Manages the CSS styles of the Flutter View.
class StyleManager {
  static const String defaultFontStyle = 'normal';
  static const String defaultFontWeight = 'normal';
  static const double defaultFontSize = 14;
  static const String defaultFontFamily = 'sans-serif';
  static const String defaultCssFont = '$defaultFontStyle $defaultFontWeight ${defaultFontSize}px $defaultFontFamily';

  static void attachGlobalStyles({
    required DomNode node,
    required String styleId,
    required String? styleNonce,
    required String cssSelectorPrefix,
  }) {
    final DomHTMLStyleElement styleElement = createDomHTMLStyleElement(styleNonce);
    styleElement.id = styleId;
    // The style element must be appended to the DOM, or its `sheet` will be null later.
    node.appendChild(styleElement);
    applyGlobalCssRulesToSheet(
      styleElement,
      defaultCssFont: StyleManager.defaultCssFont,
      cssSelectorPrefix: cssSelectorPrefix,
    );
  }

  static void styleSceneHost(
    DomElement sceneHost, {
    bool debugShowSemanticsNodes = false,
  }) {
    assert(sceneHost.tagName.toLowerCase() == DomManager.sceneHostTagName.toLowerCase());
    // Don't allow the scene to receive pointer events.
    sceneHost.style.pointerEvents = 'none';
    // When debugging semantics, make the scene semi-transparent so that the
    // semantics tree is more prominent.
    if (debugShowSemanticsNodes) {
      sceneHost.style.opacity = '0.3';
    }
  }

  static void styleSemanticsHost(
    DomElement semanticsHost,
    double devicePixelRatio,
  ) {
    assert(semanticsHost.tagName.toLowerCase() == DomManager.semanticsHostTagName.toLowerCase());
    semanticsHost.style
      ..position = 'absolute'
      ..transformOrigin = '0 0 0';
    scaleSemanticsHost(semanticsHost, devicePixelRatio);
  }

  /// The framework specifies semantics in physical pixels, but CSS uses
  /// logical pixels. To compensate, an inverse scale is injected at the root
  /// level.
  static void scaleSemanticsHost(
    DomElement semanticsHost,
    double devicePixelRatio,
  ) {
    assert(semanticsHost.tagName.toLowerCase() == DomManager.semanticsHostTagName.toLowerCase());
    semanticsHost.style.transform = 'scale(${1 / devicePixelRatio})';
  }
}
