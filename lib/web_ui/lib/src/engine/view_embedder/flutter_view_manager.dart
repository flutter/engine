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
    final String? viewIdAttribute = viewRoot?.getAttribute(GlobalHtmlAttributes.flutterViewIdAttributeName);
    final int? viewId = viewIdAttribute == null ? null : int.parse(viewIdAttribute);
    return viewId == null ? null : _viewData[viewId];
  }

  /// Safely manages focus when blurring and optionally removing a DOM element.
  ///
  /// This function ensures the blur operation doesn't disrupt the framework's view focus management.
  ///
  /// * [removeElement] controls whether the element is removed from the DOM after being blurred.
  /// * [delayed] controls whether the engine will be given the opportunity to focus on another element first.
  void safelyBlurElement(DomElement element, {bool removeElement = false, bool delayed = true}) {
    final EngineFlutterView? view = findViewForElement(element);

    void blur() {
      // If by the time the timer fired the focused element is no longer the
      // editing element whose editing session was disabled, there's no need to
      // move the focus, as it is likely that another widget already took the
      // focus.
      if (element == domDocument.activeElement) {
        view?.dom.rootElement.focusWithoutScroll();
      }
      if (removeElement) {
        element.remove();
      }
    }

    if (delayed) {
      Timer(Duration.zero, blur);
    } else {
      blur();
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
