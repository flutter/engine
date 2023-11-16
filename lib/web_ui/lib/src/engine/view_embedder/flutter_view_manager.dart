// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/js_interop/js_app.dart';

/// Encapsulates the `viewData` Map, and automates the creation of `viewId`s.
class FlutterViewManager {
  /// Creates a FlutterViewManager with a [initialViewId].
  ///
  /// This defaults to [kImplicitViewId].
  FlutterViewManager({int initialViewId = kImplicitViewId}) : _nextViewId = initialViewId;

  // The counter from which viewIds are generated. Defaults to [kImplicitViewId].
  int _nextViewId;
  final Map<int, EngineFlutterView> _viewData = <int, EngineFlutterView>{};
  final Map<int, JsFlutterViewOptions> _jsViewOptions = <int, JsFlutterViewOptions>{};

  /// Exposes all the [EngineFlutterView]s registered so far.
  Iterable<EngineFlutterView> get views => _viewData.values;

  /// Retrieves an [EngineFlutterView] by its `viewId`.
  EngineFlutterView? operator[](int viewId) {
    return _viewData[viewId];
  }

  /// Creates (and stores) a view from its [JsFlutterViewOptions].
  ///
  /// Returns a `viewId` that is automatically assigned by this method.
  int createView({ required JsFlutterViewOptions options }) {
    assert(!_viewData.containsKey(_nextViewId)); // Adding the same view twice?

    final int viewId = _nextViewId++;

    _viewData[viewId] = EngineFlutterView.fromJsOptions(viewId, options);
    _jsViewOptions[viewId] = options;

    return viewId;
  }

  /// Returns the [JsFlutterViewOptions] associated to `viewId`.
  ///
  /// This is useful for plugins and apps that need this information, and can
  /// be exposed through a method in ui_web.
  JsFlutterViewOptions? getOptions(int viewId) {
    return _jsViewOptions[viewId];
  }

  /// Unregisters [viewId].
  ///
  /// Returns its [JsFlutterViewOptions] (if any).
  JsFlutterViewOptions? unregisterView(int viewId) {
    _viewData.remove(viewId);
    return _jsViewOptions.remove(viewId);
  }

  /// Sets the "implicit view".
  int setImplicitView(EngineFlutterView view) {
    // assert(!configuration.multiViewEnabled);
    assert(view.viewId == kImplicitViewId);
    assert(!_viewData.containsKey(view.viewId));

    _nextViewId = view.viewId + 1; // Increment nextViewId, for prototyping.

    _viewData[view.viewId] = view;
    return view.viewId;
  }

  // Have a method to call dimensionsChanged when _viewData changes!
}
