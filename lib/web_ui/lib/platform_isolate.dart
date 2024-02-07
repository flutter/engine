// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

/// Methods for constructing [Isolate]s that run on the Flutter platform thread.
///
/// This is an experimental API. It may be changed or removed in future versions
/// based on user feedback.
abstract final class PlatformIsolate {
  /// Creates and spawns an isolate that shares the same code as the current
  /// isolate. The spawned isolate runs on the Flutter platform thread.
  ///
  /// This method can only be invoked from the main isolate.
  ///
  /// See [Isolate.spawn] for details.
  static Future<Isolate> spawn<T>(void Function(T) entryPoint, T message,
      {bool errorsAreFatal = true,
      SendPort? onExit,
      SendPort? onError,
      String? debugName}) => throw UnsupportedError();

  /// Runs [computation] in a new isolate on the platform thread and returns the
  /// result.
  ///
  /// This method can only be invoked from the main isolate.
  ///
  /// See [Isolate.run] for details.
  static FutureOr<R> run<R>(FutureOr<R> Function() computation,
      {String? debugName}) => computation();

  /// Returns whether the current isolate is running on the platform thread.
  external static bool isRunningOnPlatformThread() => true;
}
