// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
class CallbackHandle {
  /// Stub implementation. See docs in `../ui/`.
  CallbackHandle.fromRawHandle(this._handle)
      : assert(_handle != null, "'_handle' must not be null.");

  final int _handle;

  /// Stub implementation. See docs in `../ui/`.
  int toRawHandle() => _handle;

  @override
  bool operator ==(dynamic other) {
    if (runtimeType != other.runtimeType)
      return false;
    final CallbackHandle typedOther = other;
    return _handle == typedOther._handle;
  }

  @override
  int get hashCode => _handle.hashCode;
}

/// Stub implementation. See docs in `../ui/`.
class PluginUtilities {
  // This class is only a namespace, and should not be instantiated or
  // extended directly.
  factory PluginUtilities._() => null;

  /// Stub implementation. See docs in `../ui/`.
  static CallbackHandle getCallbackHandle(Function callback) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  static Function getCallbackFromHandle(CallbackHandle handle) {
   throw UnimplementedError();
  }
}
