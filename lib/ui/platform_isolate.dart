// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

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
FutureOr<R> runInPlatformThread<R>(FutureOr<R> Function() computation) {
  if (_platformRunnerSendPort == null) {
    final Completer<SendPort> sendPortCompleter = Completer<SendPort>();
    _platformRunnerSendPort = sendPortCompleter.future;
    final RawReceivePort portReceiver = _receiveOne((SendPort port) {
      Isolate.current.addOnExitListener(port);
      sendPortCompleter.complete(port);
    });
    portReceiver.keepIsolateAlive = false;
    final RawReceivePort onExit = _receiveOne((_) {
      // This shouldn't really happen, since Isolate.exit() is disabled on the
      // platform isolate, the pause and terminate capabilities aren't provided
      // to the parent isolate, and errors are fatal is false. But if the
      // isolate does shutdown unexpectedly, clear the singleton so we can
      // create another.
      _platformRunnerSendPort = null;
    });
    onExit.keepIsolateAlive = false;
    try {
      _spawn(_remoteRun, portReceiver.sendPort, onExit: onExit.sendPort);
    } on Object {
      portReceiver.close();
      onExit.close();
      rethrow;
    }
  }

  final Completer<R> resultCompleter = Completer<R>();
  final RawReceivePort resultPort = _receiveOne(
      ((R? result, Object? remoteError, Object? remoteStack)? response) {
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
  });

  _platformRunnerSendPort!
      .then((SendPort port) => port.send((computation, resultPort.sendPort)));
  return resultCompleter.future;
}

Future<SendPort>? _platformRunnerSendPort;

Future<void> _remoteRun(SendPort portReceiver) async {
  final RawReceivePort computationPort = RawReceivePort();
  computationPort.handler = ((
        FutureOr<Object?> Function() computation,
        SendPort resultPort
      )? message) async {
    if (message == null) {
      // The parent isolate has shutdown. Allow this isolate to shutdown.
      computationPort..keepIsolateAlive = false;
      return;
    }
    final (FutureOr<Object?> Function() computation, SendPort resultPort) =
        message;
    try {
      final FutureOr<Object?> potentiallyAsyncResult = computation();
      Object? result;
      if (potentiallyAsyncResult is Future<Object?>) {
        result = await potentiallyAsyncResult;
      } else {
        result = potentiallyAsyncResult;
      }
      resultPort.send((result, null, null));
    } catch (e, s) {
      // If sending fails, the error becomes an uncaught error.
      resultPort.send((null, e, s));
    }
  };
  portReceiver.send(computationPort.sendPort);
}

Future<Isolate> _spawn<T>(void Function(T) entryPoint, T message,
    {SendPort? onExit}) {
  final Completer<Isolate> isolateCompleter = Completer<Isolate>();
  final RawReceivePort isolateReadyPort = _receiveOne((Object readyMessage) {
    if (readyMessage is _PlatformIsolateReadyMessage) {
      final Isolate isolate = Isolate(readyMessage.controlPort)
        ..setErrorsFatal(false);
      if (onExit != null) {
        isolate.addOnExitListener(onExit);
      }

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
  });
  try {
    _nativeSpawn(
        _platformIsolateMain<T>, isolateReadyPort.sendPort, 'PlatformIsolate');
  } on Object {
    isolateReadyPort.close();
    rethrow;
  }
  return isolateCompleter.future;
}

void _platformIsolateMain<T>(SendPort isolateReadyPort) {
  final RawReceivePort entryPointPort =
      _receiveOne(((void Function(T), T) entryPointAndMessage) {
    final (void Function(T) entryPoint, T message) = entryPointAndMessage;
    entryPoint(message);
  });
  final Isolate isolate = Isolate.current;
  isolateReadyPort.send(_PlatformIsolateReadyMessage(
      isolate.controlPort, entryPointPort.sendPort));
}

@Native<Void Function(Handle, Handle, Handle)>(
    symbol: 'PlatformIsolateNativeApi::Spawn')
external void _nativeSpawn(
    Function entryPoint, SendPort isolateReadyPort, String debugName);

/// Returns whether the current isolate is running in the platform thread.
@Native<Bool Function()>(
    symbol: 'PlatformIsolateNativeApi::IsRunningInPlatformThread')
external bool isRunningInPlatformThread();

RawReceivePort _receiveOne<T>(void Function(T) then) {
  final RawReceivePort port = RawReceivePort();
  port.handler = (T value) {
    port.close();
    then(value);
  };
  return port;
}

class _PlatformIsolateReadyMessage {
  _PlatformIsolateReadyMessage(this.controlPort, this.entryPointPort);

  final SendPort controlPort;
  final SendPort entryPointPort;
}
