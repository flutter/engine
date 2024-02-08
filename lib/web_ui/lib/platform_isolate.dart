// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Runs [computation] in the platform thread and returns the result.
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
FutureOr<R> runInPlatformThread<R>(FutureOr<R> Function() computation) =>
    computation();

/// Returns whether the current isolate is running in the platform thread.
bool isRunningInPlatformThread() => true;
