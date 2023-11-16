// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@JS()
library js_app;

import 'dart:js_interop';

import 'package:ui/src/engine.dart';

/// The JS bindings for the object that's set as `window.flutterConfiguration`.
@JS()
@anonymous
@staticInterop
class JsFlutterViewOptions {
  external factory JsFlutterViewOptions();
}

/// The attributes of the [JsFlutterViewOptions] object.
extension JsFlutterViewOptionsExtension on JsFlutterViewOptions {
  external DomElement? get hostElement;

  @JS('initialData')
  external JSObject? get _initialData;
  Object? get initialData => _initialData?.toObjectDeep;
}

/// A class that exposes the public API of a running Flutter Web App running.
@JS()
@anonymous
@staticInterop
abstract class FlutterApp {
  factory FlutterApp({
    required AddFlutterViewFn addView,
    required RemoveFlutterViewFn removeView,
  }) => FlutterApp._(
      addView: ((JsFlutterViewOptions options) => futureToPromise(addView(options) as Future<JSAny>)).toJS,
      removeView: ((int id) => futureToPromise(removeView(id) as Future<JSObject>)).toJS,
    );
  external factory FlutterApp._({
    required JSFunction addView,
    required JSFunction removeView,
  });
}

/// Typedef for the function that adds a new view to the app.
///
/// Returns the ID of the newly created view.
typedef AddFlutterViewFn = Future<int> Function(JsFlutterViewOptions);

/// Typedef for the function that removes a view from the app.
///
/// Returns the configuration used to create the view.
typedef RemoveFlutterViewFn = Future<JsFlutterViewOptions?> Function(int);
