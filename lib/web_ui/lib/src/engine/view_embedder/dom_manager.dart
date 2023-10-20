// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../embedder.dart';

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
  DomManager.fromFlutterViewEmbedderDEPRECATED(FlutterViewEmbedder embedder)
      : rootElement = embedder.flutterViewElementDEPRECATED,
        platformViewsHost = embedder.glassPaneElementDEPRECATED,
        renderingHost = embedder.glassPaneShadowDEPRECATED,
        textEditingHost = embedder.textEditingHostNodeDEPRECATED,
        semanticsHost = embedder.semanticsHostElementDEPRECATED;

  /// The root DOM element for the entire Flutter View.
  ///
  /// This is where input events are captured, such as pointer events.
  ///
  /// If semantics is enabled, this element also contains the semantics DOM tree,
  /// which captures semantics input events.
  final DomElement rootElement;

  /// Hosts all platform view elements.
  final DomElement platformViewsHost;

  /// Hosts all rendering elements and canvases.
  final DomShadowRoot renderingHost;

  /// Hosts all text editing elements.
  final DomElement textEditingHost;

  /// Hosts the semantics tree.
  ///
  /// This element is in front of the [renderingHost] and [platformViewsHost].
  /// Otherwise, the phone will disable focusing by touch, only by tabbing
  /// around the UI.
  final DomElement semanticsHost;
}
