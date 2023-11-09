// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../engine.dart' show buildMode, renderer;
import 'browser_detection.dart';
import 'configuration.dart';
import 'dom.dart';
import 'keyboard_binding.dart';
import 'platform_dispatcher.dart';
import 'pointer_binding.dart';
import 'text_editing/text_editing.dart';
import 'view_embedder/dimensions_provider/dimensions_provider.dart';
import 'view_embedder/embedding_strategy/embedding_strategy.dart';
import 'view_embedder/style_manager.dart';
import 'window.dart';

/// Controls the placement and lifecycle of a Flutter view on the web page.
///
/// Manages several top-level elements that host Flutter-generated content,
/// including:
///
/// - [flutterViewElement], the root element of a Flutter view.
/// - [glassPaneElement], the glass pane element that hosts the shadowDOM.
/// - [glassPaneShadow], the shadow root used to isolate Flutter-rendered
///   content from the surrounding page content, including from the platform
///   views.
/// - [sceneElement], the element that hosts Flutter layers and pictures, and
///   projects platform views.
/// - [sceneHostElement], the anchor that provides a stable location in the DOM
///   tree for the [sceneElement].
/// - [semanticsHostElement], hosts the ARIA-annotated semantics tree.
///
/// This class is currently a singleton, but it'll possibly need to morph to have
/// multiple instances in a multi-view scenario. (One ViewEmbedder per FlutterView).
class FlutterViewEmbedder {
  /// Creates a FlutterViewEmbedder.
  ///
  /// The incoming [hostElement] parameter specifies the root element in the DOM
  /// into which Flutter will be rendered.
  ///
  /// The hostElement is abstracted by an [EmbeddingStrategy] instance, which has
  /// different behavior depending on the `hostElement` value:
  ///
  /// - A `null` `hostElement` will cause Flutter to take over the whole page.
  /// - A non-`null` `hostElement` will render flutter inside that element.
  FlutterViewEmbedder({DomElement? hostElement})
      : _embeddingStrategy =
            EmbeddingStrategy.create(hostElement: hostElement) {
    // Configure the EngineWindow so it knows how to measure itself.
    // TODO(dit): Refactor ownership according to new design, https://github.com/flutter/flutter/issues/117098
    window.configureDimensionsProvider(DimensionsProvider.create(
      hostElement: hostElement,
    ));

    reset();
  }

  /// Abstracts all the DOM manipulations required to embed a Flutter app in an user-supplied `hostElement`.
  final EmbeddingStrategy _embeddingStrategy;

  DomElement get _sceneHostElement => window.dom.sceneHost;

  /// A child element of body outside the shadowroot that hosts
  /// global resources such svg filters and clip paths when using webkit.
  DomElement? _resourcesHost;

  DomElement get _semanticsHostElement => window.dom.semanticsHost;

  /// The last scene element rendered by the [render] method.
  DomElement? get sceneElement => _sceneElement;
  DomElement? _sceneElement;

  /// Don't unnecessarily move DOM nodes around. If a DOM node is
  /// already in the right place, skip DOM mutation. This is both faster and
  /// more correct, because moving DOM nodes loses internal state, such as
  /// text selection.
  void addSceneToSceneHost(DomElement? sceneElement) {
    if (sceneElement != _sceneElement) {
      _sceneElement?.remove();
      _sceneElement = sceneElement;
      _sceneHostElement.append(sceneElement!);
    }
  }

  DomElement get _flutterViewElement => window.dom.rootElement;
  DomShadowRoot get _glassPaneShadow => window.dom.renderingHost;

  void reset() {
    // How was the current renderer selected?
    const String rendererSelection = FlutterConfiguration.flutterWebAutoDetect
        ? 'auto-selected'
        : 'requested explicitly';

    // Initializes the embeddingStrategy so it can host a single-view Flutter app.
    _embeddingStrategy.initialize(
      hostElementAttributes: <String, String>{
        'flt-renderer': '${renderer.rendererTag} ($rendererSelection)',
        'flt-build-mode': buildMode,
        // TODO(mdebbar): Disable spellcheck until changes in the framework and
        // engine are complete.
        'spellcheck': 'false',
      },
    );

    // TODO(mdebbar): Inserting into the DOM should be done by EngineFlutterView.

    // This must be attached to the DOM now, so the engine can create a host
    // node (ShadowDOM or a fallback) next.
    //
    // The embeddingStrategy will take care of cleaning up the glassPane on hot
    // restart.
    _embeddingStrategy.attachGlassPane(_flutterViewElement);

    renderer.reset(this);

    // TODO(mdebbar): Move these to `engine/initialization.dart`.

    KeyboardBinding.initInstance();
    PointerBinding.initInstance(
      _flutterViewElement,
      KeyboardBinding.instance!.converter,
    );

    window.onResize.listen(_metricsDidChange);
  }

  /// Called immediately after browser window metrics change.
  ///
  /// When there is a text editing going on in mobile devices, do not change
  /// the physicalSize, change the [window.viewInsets]. See:
  /// https://api.flutter.dev/flutter/dart-ui/FlutterView/viewInsets.html
  /// https://api.flutter.dev/flutter/dart-ui/FlutterView/physicalSize.html
  ///
  /// Note: always check for rotations for a mobile device. Update the physical
  /// size if the change is caused by a rotation.
  void _metricsDidChange(ui.Size? newSize) {
    StyleManager.scaleSemanticsHost(
      _semanticsHostElement,
      window.devicePixelRatio,
    );
    // TODO(dit): Do not computePhysicalSize twice, https://github.com/flutter/flutter/issues/117036
    if (isMobile && !window.isRotation() && textEditing.isEditing) {
      window.computeOnScreenKeyboardInsets(true);
      EnginePlatformDispatcher.instance.invokeOnMetricsChanged();
    } else {
      window.computePhysicalSize();
      // When physical size changes this value has to be recalculated.
      window.computeOnScreenKeyboardInsets(false);
      EnginePlatformDispatcher.instance.invokeOnMetricsChanged();
    }
  }

  /// Add an element as a global resource to be referenced by CSS.
  ///
  /// This call create a global resource host element on demand and either
  /// place it as first element of body(webkit), or as a child of
  /// glass pane element for other browsers to make sure url resolution
  /// works correctly when content is inside a shadow root.
  void addResource(DomElement element) {
    final bool isWebKit = browserEngine == BrowserEngine.webkit;
    if (_resourcesHost == null) {
      final DomElement resourcesHost = domDocument
          .createElement('flt-svg-filters')
        ..style.visibility = 'hidden';
      if (isWebKit) {
        // The resourcesHost *must* be a sibling of the glassPaneElement.
        _embeddingStrategy.attachResourcesHost(resourcesHost,
            nextTo: _flutterViewElement);
      } else {
        _glassPaneShadow.insertBefore(resourcesHost, _glassPaneShadow.firstChild);
      }
      _resourcesHost = resourcesHost;
    }
    _resourcesHost!.append(element);
  }

  /// Removes a global resource element.
  void removeResource(DomElement? element) {
    if (element == null) {
      return;
    }
    assert(element.parentNode == _resourcesHost);
    element.remove();
  }
}

/// The embedder singleton.
///
/// [ensureFlutterViewEmbedderInitialized] must be called prior to calling this
/// getter.
FlutterViewEmbedder get flutterViewEmbedder {
  final FlutterViewEmbedder? embedder = _flutterViewEmbedder;
  assert(() {
    if (embedder == null) {
      throw StateError(
          'FlutterViewEmbedder not initialized. Call `ensureFlutterViewEmbedderInitialized()` '
          'prior to calling the `flutterViewEmbedder` getter.');
    }
    return true;
  }());
  return embedder!;
}

FlutterViewEmbedder? _flutterViewEmbedder;

/// Initializes the [FlutterViewEmbedder], if it's not already initialized.
FlutterViewEmbedder ensureFlutterViewEmbedderInitialized() {
  // FlutterViewEmbedder needs the implicit view to be initialized because it
  // uses some of its methods e.g. `configureDimensionsProvider`, `onResize`.
  ensureImplicitViewInitialized();
  return _flutterViewEmbedder ??=
      FlutterViewEmbedder(hostElement: configuration.hostElement);
}
