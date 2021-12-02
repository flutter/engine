// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Provides a compile time constant to customize flutter framework and other
/// users of ui engine for web runtime.
const bool isWeb = true;

/// Web specific SMI. Used by bitfield. The 0x3FFFFFFFFFFFFFFF used on VM
/// is not supported on Web platform.
const int kMaxUnsignedSMI = -1;

void webOnlyInitializeEngine() {
  engine.initializeEngine();
}

void webOnlySetPluginHandler(Future<void> Function(String, ByteData?, PlatformMessageResponseCallback?) handler) {
  engine.pluginMessageCallHandler = handler;
}

// TODO(yjbanov): The code below was temporarily moved from lib/web_ui/lib/src/engine/platform_views.dart
//                during the NNBD migration so that `dart:ui` does not have to export `dart:_engine`. NNBD
//                does not allow exported non-migrated libraries from migrated libraries. When `dart:_engine`
//                is migrated, we can move it back.

/// A function which takes a unique `id` and creates an HTML element.
typedef PlatformViewFactory = html.Element Function(int viewId);

/// A registry for factories that create platform views.
class PlatformViewRegistry {
  /// Register [viewTypeId] as being creating by the given [factory].
  bool registerViewFactory(String viewTypeId, PlatformViewFactory viewFactory,
      {bool isVisible = true}) {
    // TODO(web): Deprecate this once there's another way of calling `registerFactory` (js interop?)
    return engine.platformViewManager
        .registerFactory(viewTypeId, viewFactory, isVisible: isVisible);
  }
}

/// The platform view registry for this app.
final PlatformViewRegistry platformViewRegistry = PlatformViewRegistry();

// TODO(yjbanov): remove _Callback, _Callbacker, and _futurize. They are here only
//                because the analyzer wasn't able to infer the correct types during
//                NNBD migration.
typedef _Callback<T> = void Function(T result);
typedef _Callbacker<T> = String? Function(_Callback<T> callback);
Future<T> _futurize<T>(_Callbacker<T> callbacker) {
  final Completer<T> completer = Completer<T>.sync();
  final String? error = callbacker((T t) {
    if (t == null) {
      completer.completeError(Exception('operation failed'));
    } else {
      completer.complete(t);
    }
  });
  if (error != null) {
    throw Exception(error);
  }
  return completer.future;
}
