// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

class PlatformIsolate {
  static Future<Isolate> spawn<T>(
      void entryPoint(T message), T message,
      {/*bool paused = false,
      bool errorsAreFatal = true,
      SendPort? onExit,
      SendPort? onError,*/
      String? debugName}) {
    final isolateCompleter = Completer<Isolate>();
    final isolatePort = RawReceivePort();
    isolatePort.handler = (message) {
      isolatePort.close();
      print("PlatformIsolate has been spawned: $message");

      if (message is _PlatformIsolateReadyMessage) {
        isolateCompleter.complete(new Isolate(
            message.controlPort,
            pauseCapability: message.pauseCapability,
            terminateCapability: message.terminateCapability));
      } else if (message is String) {
        // We encountered an error while starting the new isolate.
        isolateCompleter.completeError(new IsolateSpawnException(
            'Unable to spawn isolate: $message'));
      } else {
        // This shouldn't happen.
        isolateCompleter.completeError(new IsolateSpawnException(
            "Internal error: unexpected format for ready message: "
            "'$message'"));
      }
    };
    _spawn(_wrapEntryPoint(entryPoint, message, isolatePort.sendPort),
        debugName ?? "PlatformIsolate");
    return isolateCompleter.future;
  }

  static void Function() _wrapEntryPoint<T>(
      void entryPoint(T message), T message, SendPort sendPort) {
    return () {
      final isolate = Isolate.current;
      sendPort.send(_PlatformIsolateReadyMessage(
          isolate.controlPort, isolate.pauseCapability,
          isolate.terminateCapability));
      entryPoint(message);
    };
  }

  @Native<Void Function(Handle, Handle)>(symbol: 'PlatformIsolateNativeApi::Spawn')
  external static void _spawn(Function entryPoint, String debugName);

  static Future<R> run<R>(FutureOr<R> computation(), {String? debugName}) {
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
      if (result == null) {
        if (remoteStack is StackTrace) {
          // Typed error.
          resultCompleter.completeError(remoteError!, remoteStack);
        } else {
          // onError handler message, uncaught async error.
          // Both values are strings, so calling `toString` is efficient.
          final error =
              RemoteError(remoteError!.toString(), remoteStack!.toString());
          resultCompleter.completeError(error, error.stackTrace);
        }
      } else {
        resultCompleter.complete(result);
      }
    };
    try {
      /*PlatformIsolate.spawn(_RemoteRunner._remoteExecute,
              _RemoteRunner<R>(computation, resultPort.sendPort),
              onError: resultPort.sendPort,
              onExit: resultPort.sendPort,
              errorsAreFatal: true,
              debugName: debugName)
          .then<void>((_) {}, onError: (error, stack) {
        // Sending the computation failed asynchronously.
        // Do not expect a response, report the error asynchronously.
        resultPort.close();
        result.completeError(error, stack);
      });*/
      PlatformIsolate.spawn(_remoteRun, (computation, resultPort.sendPort));
    } on Object {
      // Sending the computation failed synchronously.
      // This is not expected to happen, but if it does,
      // the synchronous error is respected and rethrown synchronously.
      resultPort.close();
      rethrow;
    }
    return resultCompleter.future;
  }

  static void _remoteRun<R>(
      (FutureOr<R> Function() computation, SendPort sendPort) args) async {
    final (computation, sendPort) = args;
    late final result;
    try {
      final potentiallyAsyncResult = computation();
      if (potentiallyAsyncResult is Future<R>) {
        result = await potentiallyAsyncResult;
      } else {
        result = potentiallyAsyncResult;
      }
    } catch (e, s) {
      // If sending fails, the error becomes an uncaught error.
      //Isolate.exit(resultPort, (e, s));
      sendPort.send((null, e, s));
    }
    //Isolate.exit(resultPort, (result));
    sendPort.send((result, null, null));
  }

  @Native<Uint32 Function()>(symbol: 'PlatformIsolateNativeApi::GetCurrentThreadId')
  external static int getCurrentThreadId();
}

class _PlatformIsolateReadyMessage {
  final SendPort controlPort;
  final Capability? pauseCapability;
  final Capability? terminateCapability;

  _PlatformIsolateReadyMessage(
      this.controlPort, this.pauseCapability, this.terminateCapability);
}
