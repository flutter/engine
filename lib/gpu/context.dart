// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of gpu;

class GpuContextException implements Exception {
  GpuContextException(this.message);
  String message;

  @override
  String toString() {
    return 'GpuContextException: $message';
  }
}

/// A handle to a graphics context. Used to create and manage GPU resources.
///
/// To obtain the default graphics context, use [getGpuContext].
class GpuContext extends NativeFieldWrapperClass1 {
  /// Creates a new graphics context that corresponds to the default Impeller
  /// context.
  GpuContext._createDefault() {
    final String error = _initializeDefault();
    if (error.isNotEmpty) {
      throw GpuContextException(error);
    }
  }

  /// Associates the default Impeller context with this GpuContext.
  @Native<Handle Function(Handle)>(
      symbol: 'InternalFlutterGpu_GpuContext_InitializeDefault')
  external String _initializeDefault();
}

GpuContext? _defaultGpuContext;

/// Returns the default graphics context.
GpuContext getGpuContext() {
  _defaultGpuContext ??= GpuContext._createDefault();
  return _defaultGpuContext!;
}
