// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import 'dart:js_interop';
import 'package:ui/src/engine.dart';

/// The proxy instance for the internal `viewManager` of this app.
final FlutterViewManagerProxy viewsProxy = FlutterViewManagerProxy();

/// Proxy class for the internal [FlutterViewManager].
class FlutterViewManagerProxy {
  /// Returns the `initialData` configuration value passed from JS when `viewId` was added.
  ///
  /// Developers can access the initial data from Dart in two ways:
  ///  * Defining their own `staticInterop` class that describes what you're
  ///    passing to your views, to retain type safety on Dart (preferred).
  ///  * Calling [NullableUndefineableJSAnyExtension.dartify] and accessing the
  ///    returned object as if it were a [Map] (not recommended).
  JSAny? getInitialData(int viewId) {
    return EnginePlatformDispatcher.instance.viewManager.getOptions(viewId)?.initialData;
  }
}
