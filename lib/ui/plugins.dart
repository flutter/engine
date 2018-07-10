// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

/// Functionality for Flutter plugin authors.
abstract class PluginUtilities {
  /// Get a handle to a named top-level or static callback function which can
  /// be easily passed between isolates.
  ///
  /// `callback` must not be null.
  ///
  /// Returns an integer that can be provided to
  /// [PluginUtilities.getCallbackFromHandle] to retrieve a tear-off of the
  /// original callback. If `callback` is not a top-level or static function,
  /// null is returned.
  static int getCallbackHandle(Function callback) {
    assert(callback != null, "'callback' must not be null.");
    return _getCallbackHandle(callback);
  }

  /// Get a tear-off of a named top-level or static callback represented by a
  /// handle.
  ///
  /// `handle` must not be null.
  ///
  /// If `handle` is not a valid handle returned by
  /// [PluginUtilities.getCallbackHandle], null is returned. Otherwise, a
  /// tear-off of the callback associated with `handle` is returned.
  static Function getCallbackFromHandle(int handle) {
    assert(handle != null, "'handle' must not be null.");
    return _getCallbackFromHandle(handle);
  }
}
