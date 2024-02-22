// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Runs [computation] in the platform thread and returns the result.
///
/// Internally this creates an isolate in the platform thread that will be
/// reused for subsequent [runInPlatformThread] calls. This means that global
/// state is maintained in that isolate between calls.
///
/// The [computation] and any state it captures will be sent to that isolate.
/// See [SendPort.send] for information about what types can be sent.
///
/// If [computation] is asynchronous (returns a `Future<R>`) then
/// that future is awaited in the new isolate, completing the entire
/// asynchronous computation, before returning the result.
///
/// If [computation] throws, the `Future` returned by this function completes
/// with that error.
///
/// The [computation] function and its result (or error) must be
/// sendable between isolates. Objects that cannot be sent include open
/// files and sockets (see [SendPort.send] for details).
///
/// This method can only be invoked from the main isolate.
Future<R> runInPlatformThread<R>(FutureOr<R> Function() computation) =>
    Future<R>(computation);

/// Returns whether the current isolate is running in the platform thread.
bool isRunningInPlatformThread() => true;
