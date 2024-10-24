// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import 'dart:async';
import 'package:ui/src/engine.dart';

/// Encapsulates view objects, and their optional metadata indexed by `viewId`.
class FlutterViewManager {
  FlutterViewManager(this._dispatcher);

  final EnginePlatformDispatcher _dispatcher;

  // A map of EngineFlutterViews indexed by their viewId.
  final Map<int, EngineFlutterView> _viewData = <int, EngineFlutterView>{};

  // A map of (optional) JsFlutterViewOptions, indexed by their viewId.
  final Map<int, JsFlutterViewOptions> _jsViewOptions =
      <int, JsFlutterViewOptions>{};

  // The controller of the [onViewCreated] stream.
  final StreamController<int> _onViewCreatedController =
      StreamController<int>.broadcast(sync: true);

  // The controller of the [onViewDisposed] stream.
  final StreamController<int> _onViewDisposedController =
      StreamController<int>.broadcast(sync: true);

  /// A stream of viewIds that will fire when a view is created.
  Stream<int> get onViewCreated => _onViewCreatedController.stream;

  /// A stream of viewIds that will fire when a view is disposed.
  Stream<int> get onViewDisposed => _onViewDisposedController.stream;

  /// Exposes all the [EngineFlutterView]s registered so far.
  Iterable<EngineFlutterView> get views => _viewData.values;

  /// Retrieves an [EngineFlutterView] by its `viewId`.
  EngineFlutterView? operator [](int viewId) {
    return _viewData[viewId];
  }

  EngineFlutterView createAndRegisterView(
    JsFlutterViewOptions jsViewOptions,
  ) {
    final EngineFlutterView view = EngineFlutterView(
      _dispatcher,
      jsViewOptions.hostElement,
      viewConstraints: jsViewOptions.viewConstraints,
    );
    registerView(view, jsViewOptions: jsViewOptions);
    return view;
  }

  /// Stores a [view] and its (optional) [jsViewOptions], indexed by `viewId`.
  ///
  /// Returns the registered [view].
  EngineFlutterView registerView(
    EngineFlutterView view, {
    JsFlutterViewOptions? jsViewOptions,
  }) {
    final int viewId = view.viewId;
    assert(!_viewData.containsKey(viewId)); // Adding the same view twice?

    // Store the view, and the jsViewOptions, if any...
    _viewData[viewId] = view;
    if (jsViewOptions != null) {
      _jsViewOptions[viewId] = jsViewOptions;
    }
    _onViewCreatedController.add(viewId);

    return view;
  }

  JsFlutterViewOptions? disposeAndUnregisterView(int viewId) {
    final EngineFlutterView? view = _viewData[viewId];
    if (view == null) {
      return null;
    }
    final JsFlutterViewOptions? options = unregisterView(viewId);
    view.dispose();
    return options;
  }

  /// Un-registers [viewId].
  ///
  /// Returns its [JsFlutterViewOptions] (if any).
  JsFlutterViewOptions? unregisterView(int viewId) {
    _viewData.remove(viewId);
    final JsFlutterViewOptions? jsViewOptions = _jsViewOptions.remove(viewId);
    _onViewDisposedController.add(viewId);
    return jsViewOptions;
  }

  /// Returns the [JsFlutterViewOptions] associated to `viewId` (if any).
  ///
  /// This is useful for plugins and apps that need this information, and can
  /// be exposed through a method in ui_web.
  JsFlutterViewOptions? getOptions(int viewId) {
    return _jsViewOptions[viewId];
  }

  /// Returns the DOM element in which the Flutter view associated to [viewId] is embedded.
  DomElement? getHostElement(int viewId) {
    return _viewData[viewId]?.embeddingStrategy.hostElement;
  }

  EngineFlutterView? findViewForElement(DomElement? element) {
    const String viewRootSelector =
        '${DomManager.flutterViewTagName}[${GlobalHtmlAttributes.flutterViewIdAttributeName}]';
    final DomElement? viewRoot = element?.closest(viewRootSelector);
    if (viewRoot == null) {
      // `element` is not inside any flutter view.
      return null;
    }

    final String? viewIdAttribute = viewRoot.getAttribute(GlobalHtmlAttributes.flutterViewIdAttributeName);
    assert(viewIdAttribute != null, 'Located Flutter view is missing its id attribute.');

    final int? viewId = int.tryParse(viewIdAttribute!);
    assert(viewId != null, 'Flutter view id must be a valid int.');

    return _viewData[viewId];
  }

  /// Attempts to transfer focus (blur) from [element] to its
  /// [EngineFlutterView] DOM's `rootElement`.
  ///
  /// This focus "transfer" achieves two things:
  ///
  /// * Ensures the focus is preserved within the Flutter View when blurring
  ///   elements that are part of the internal DOM structure of the Flutter
  ///   app. This...
  /// * Prevents the Flutter engine from reporting bogus "blur" events from the
  ///   Flutter View because, by default, calling "blur" on an element moves the
  ///   document.currentElement to the `body` of the page.
  ///
  /// See: https://jsfiddle.net/ditman/1e2swpno for a JS-only demonstration.
  ///
  /// When [removeElement] is true, `element` will be removed from the DOM after
  /// its focus is transferred to the root of the view. This can be used to
  /// safely remove (potentially focused) element, by preserving focus within
  /// the Flutter view.
  ///
  /// When [delayed] is true, the blur operation is executed asynchronously as
  /// soon as possible (see [Timer.run]). Else it runs immediately.
  void safelyBlurElement(
    DomElement element, {
    bool removeElement = false,
    bool delayed = true
  }) {
    if (delayed) {
      Timer.run(() {
        _transferFocusToViewRoot(element, removeElement: removeElement);
      });
      return;
    }
    _transferFocusToViewRoot(element, removeElement: removeElement);
  }

  // The actual implementation of [safelyBlurElement].
  void _transferFocusToViewRoot(
    DomElement element, {
    required bool removeElement
  }) {
    // If by the time this method is called the focused element is no longer
    // `element`, there's no need to move the focus.
    //
    // This can happen when another element grabs focus when this method runs
    // "delayed".
    if (element == domDocument.activeElement) {
      final EngineFlutterView? view = findViewForElement(element);
      // Transfer the browser focus to the root element of `view`
      view?.dom.rootElement.focusWithoutScroll();
    }
    if (removeElement) {
      element.remove();
    }
  }

  void dispose() {
    // We need to call `toList()` in order to avoid concurrent modification
    // inside the loop.
    _viewData.keys.toList().forEach(disposeAndUnregisterView);
    // Let listeners receive the unregistration events from the loop above, then
    // close the streams.
    _onViewCreatedController.close();
    _onViewDisposedController.close();
  }
}
