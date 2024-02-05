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
      String? debugName}) {
    final Completer<Isolate> isolateCompleter = Completer<Isolate>();
    final RawReceivePort isolateReadyPort = RawReceivePort();
    isolateReadyPort.handler = (Object readyMessage) {
      isolateReadyPort.close();

      if (readyMessage is _PlatformIsolateReadyMessage) {
        final Isolate isolate = Isolate(readyMessage.controlPort,
            pauseCapability: readyMessage.pauseCapability,
            terminateCapability: readyMessage.terminateCapability);
        if (onError != null) {
          isolate.addErrorListener(onError);
        }
        if (onExit != null) {
          isolate.addOnExitListener(onExit);
        }
        isolate.setErrorsFatal(errorsAreFatal);

        isolateCompleter.complete(isolate);
        readyMessage.entryPointPort.send((entryPoint, message));
      } else if (readyMessage is String) {
        // We encountered an error while starting the new isolate.
        isolateCompleter.completeError(
            IsolateSpawnException('Unable to spawn isolate: $readyMessage'));
      } else {
        // This shouldn't happen.
        isolateCompleter.completeError(IsolateSpawnException(
            'Internal error: unexpected format for ready message: '
            "'$readyMessage'"));
      }
    };
    _spawn(_platformIsolateMain<T>, isolateReadyPort.sendPort,
        debugName ?? 'PlatformIsolate', errorsAreFatal);
    return isolateCompleter.future;
  }

  static void _platformIsolateMain<T>(SendPort isolateReadyPort) {
    final RawReceivePort entryPointPort = RawReceivePort();
    entryPointPort.handler = ((void Function(T), T) entryPointAndMessage) {
      entryPointPort.close();
      final (void Function(T) entryPoint, T message) = entryPointAndMessage;
      entryPoint(message);
    };
    final Isolate isolate = Isolate.current;
    isolateReadyPort.send(_PlatformIsolateReadyMessage(
        isolate.controlPort,
        isolate.pauseCapability,
        isolate.terminateCapability,
        entryPointPort.sendPort));
  }

  @Native<Void Function(Handle, Handle, Handle, Bool)>(
      symbol: 'PlatformIsolateNativeApi::Spawn')
  external static void _spawn(Function entryPoint, SendPort isolateReadyPort,
      String debugName, bool errorsAreFatal);

  /// Runs [computation] in a new isolate on the platform thread and returns the
  /// result.
  ///
  /// This method can only be invoked from the main isolate.
  ///
  /// See [Isolate.run] for details.
  static FutureOr<R> run<R>(FutureOr<R> Function() computation,
      {String? debugName}) {
    final Completer<R> resultCompleter = Completer<R>();
    final RawReceivePort resultPort = RawReceivePort();
    resultPort.handler =
        ((R? result, Object? remoteError, Object? remoteStack)? response) {
      resultPort.close();
      if (response == null) {
        // onExit handler message, isolate terminated without sending result.
        resultCompleter.completeError(
            RemoteError('Computation ended without result', ''),
            StackTrace.empty);
        return;
      }
      final (R? result, Object? remoteError, Object? remoteStack) = response;
      if (remoteStack != null) {
        if (remoteStack is StackTrace) {
          // Typed error.
          resultCompleter.completeError(remoteError!, remoteStack);
        } else {
          // onError handler message, uncaught async error.
          // Both values are strings, so calling `toString` is efficient.
          final RemoteError error =
              RemoteError(remoteError!.toString(), remoteStack.toString());
          resultCompleter.completeError(error, error.stackTrace);
        }
      } else {
        resultCompleter.complete(result);
      }
    };
    try {
      PlatformIsolate.spawn(_remoteRun<R>, (computation, resultPort.sendPort),
          debugName: debugName);
    } on Object {
      resultPort.close();
      rethrow;
    }
    return resultCompleter.future;
  }

  static Future<void> _remoteRun<R>(
      (FutureOr<R> Function() computation, SendPort resultPort) args) async {
    final (FutureOr<R> Function() computation, SendPort resultPort) = args;
    late final R result;
    try {
      final FutureOr<R> potentiallyAsyncResult = computation();
      if (potentiallyAsyncResult is Future<R>) {
        result = await potentiallyAsyncResult;
      } else {
        result = potentiallyAsyncResult;
      }
      // TODO(liamappelbe): Use Isolate.exit. At the moment this works, but logs
      // a spurious error. We need to silence that error.
      // https://github.com/flutter/flutter/issues/136314
      //Isolate.exit(resultPort, (result, null, null));
      resultPort.send((result, null, null));
    } catch (e, s) {
      // If sending fails, the error becomes an uncaught error.
      //Isolate.exit(resultPort, (null, e, s));
      resultPort.send((null, e, s));
    }
  }

  /// Returns whether the current isolate is running on the platform thread.
  @Native<Bool Function()>(
      symbol: 'PlatformIsolateNativeApi::IsRunningOnPlatformThread')
  external static bool isRunningOnPlatformThread();
}

class _PlatformIsolateReadyMessage {
  _PlatformIsolateReadyMessage(this.controlPort, this.pauseCapability,
      this.terminateCapability, this.entryPointPort);

  final SendPort controlPort;
  final Capability? pauseCapability;
  final Capability? terminateCapability;
  final SendPort entryPointPort;
}
