// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

class PlatformIsolate {
  static Future<Isolate> spawn<T>(
      void entryPoint(T message), T message,
      {bool errorsAreFatal = true,
      SendPort? onExit,
      SendPort? onError,
      String? debugName}) {
    final isolateCompleter = Completer<Isolate>();
    final isolateReadyPort = RawReceivePort();
    isolateReadyPort.handler = (readyMessage) {
      isolateReadyPort.close();

      if (readyMessage is _PlatformIsolateReadyMessage) {
        final isolate = new Isolate(
            readyMessage.controlPort,
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
        isolateCompleter.completeError(new IsolateSpawnException(
            'Unable to spawn isolate: $readyMessage'));
      } else {
        // This shouldn't happen.
        isolateCompleter.completeError(new IsolateSpawnException(
            "Internal error: unexpected format for ready message: "
            "'$readyMessage'"));
      }
    };
    _spawn(_platformIsolateMain<T>, isolateReadyPort.sendPort,
        debugName ?? "PlatformIsolate", errorsAreFatal);
    return isolateCompleter.future;
  }

  static void _platformIsolateMain<T>(SendPort isolateReadyPort) {
    final entryPointPort = RawReceivePort();
    entryPointPort.handler = (entryPointAndMessage) {
      entryPointPort.close();
      final (void Function(T) entryPoint, T message) = entryPointAndMessage;
      entryPoint(message);
    };
    final isolate = Isolate.current;
    isolateReadyPort.send(_PlatformIsolateReadyMessage(
        isolate.controlPort, isolate.pauseCapability,
        isolate.terminateCapability, entryPointPort.sendPort));
  }

  @Native<Void Function(Handle, Handle, Handle, Bool)>(symbol: 'PlatformIsolateNativeApi::Spawn')
  external static void _spawn(
      Function entryPoint, SendPort isolateReadyPort, String debugName, bool errorsAreFatal);

  static FutureOr<R> run<R>(FutureOr<R> computation(), {String? debugName}) {
    final resultCompleter = Completer<R>();
    final resultPort = RawReceivePort();
    resultPort.handler = (
        (R? result, Object? remoteError, Object? remoteStack)? response) {
      resultPort.close();
      if (response == null) {
        // onExit handler message, isolate terminated without sending result.
        resultCompleter.completeError(
            RemoteError("Computation ended without result", ""),
            StackTrace.empty);
        return;
      }
      final (result, remoteError, remoteStack) = response;
      if (remoteStack != null) {
        if (remoteStack is StackTrace) {
          // Typed error.
          resultCompleter.completeError(remoteError!, remoteStack);
        } else {
          // onError handler message, uncaught async error.
          // Both values are strings, so calling `toString` is efficient.
          final error =
              RemoteError(remoteError!.toString(), remoteStack.toString());
          resultCompleter.completeError(error, error.stackTrace);
        }
      } else {
        resultCompleter.complete(result);
      }
    };
    try {
      PlatformIsolate.spawn(_remoteRun, (computation, resultPort.sendPort),
          debugName: debugName);
    } on Object {
      resultPort.close();
      rethrow;
    }
    return resultCompleter.future;
  }

  static void _remoteRun<R>(
      (FutureOr<R> Function() computation, SendPort resultPort) args) async {
    final (computation, resultPort) = args;
    late final result;
    try {
      final potentiallyAsyncResult = computation();
      if (potentiallyAsyncResult is Future<R>) {
        result = await potentiallyAsyncResult;
      } else {
        result = potentiallyAsyncResult;
      }
      // TODO(flutter/flutter#136314): Use Isolate.exit. At the moment this
      // works, but logs a spurious error. We need to silence that error.
      //Isolate.exit(resultPort, (result, null, null));
      resultPort.send((result, null, null));
    } catch (e, s) {
      // If sending fails, the error becomes an uncaught error.
      //Isolate.exit(resultPort, (null, e, s));
      resultPort.send((null, e, s));
    }
  }

  @Native<Bool Function()>(symbol: 'PlatformIsolateNativeApi::IsRunningOnPlatformThread')
  external static bool isRunningOnPlatformThread();
}

class _PlatformIsolateReadyMessage {
  final SendPort controlPort;
  final Capability? pauseCapability;
  final Capability? terminateCapability;
  final SendPort entryPointPort;

  _PlatformIsolateReadyMessage(
      this.controlPort, this.pauseCapability, this.terminateCapability,
      this.entryPointPort);
}
