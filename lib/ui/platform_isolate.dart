// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

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
Future<R> runInPlatformThread<R>(FutureOr<R> Function() computation) {
  final SendPort? sendPort = _platformRunnerSendPort;
  if (sendPort != null) {
    return _sendComputation(sendPort, computation);
  } else {
    return _platformRunnerSendPortFuture
        .then((SendPort port) => _sendComputation(port, computation));
  }
}

SendPort? _platformRunnerSendPort;
final Future<SendPort> _platformRunnerSendPortFuture = _spawnPlatformIsolate();
final Map<int, Completer<Object?>> _pending = <int, Completer<Object?>>{};
int _nextId = 0;

Future<SendPort> _spawnPlatformIsolate() {
  final Completer<SendPort> sendPortCompleter = Completer<SendPort>();
  final RawReceivePort receiver = RawReceivePort()..keepIsolateAlive = false;
  receiver.handler = (Object? message) {
    if (message == null) {
      // This is the platform isolate's onExit handler.
      // This shouldn't really happen, since Isolate.exit is disabled, the
      // pause and terminate capabilities aren't provided to the parent
      // isolate, and errors are fatal is false. But if the isolate does
      // shutdown unexpectedly, clear the singleton so we can create another.
      // TODO(liamappelbe): Write a test that artificially hits this case, then
      // terminate all pending computations and reset the static variables.
    } else if (message is _PlatformIsolateReadyMessage) {
      _platformRunnerSendPort = message.computationPort;
      Isolate.current.addOnExitListener(message.computationPort);
      sendPortCompleter.complete(message.computationPort);
    } else if (message is _ComputationResult) {
      final Completer<Object?> resultCompleter = _pending.remove(message.id)!;
      final Object? remoteStack = message.remoteStack;
      final Object? remoteError = message.remoteError;
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
        resultCompleter.complete(message.result);
      }
    } else if (message is String) {
      // We encountered an error while starting the new isolate.
      throw IsolateSpawnException('Unable to spawn isolate: $message');
    } else {
      // This shouldn't happen.
      throw IsolateSpawnException(
          "Internal error: unexpected format for ready message: '$message'");
    }
  };
  try {
    _nativeSpawn(_platformIsolateMain<SendPort>, receiver.sendPort);
  } on Object {
    receiver.close();
    rethrow;
  }
  return sendPortCompleter.future;
}

Future<R> _sendComputation<R>(
    SendPort port, FutureOr<R> Function() computation) {
  final int id = ++_nextId;
  final Completer<R> resultCompleter = Completer<R>();
  _pending[id] = resultCompleter;
  port.send(_ComputationRequest(id, computation));
  return resultCompleter.future;
}

void _platformIsolateMain<T>(SendPort sendPort) {
  final RawReceivePort computationPort = RawReceivePort();
  computationPort.handler = (_ComputationRequest? message) {
    if (message == null) {
      // The parent isolate has shutdown. Allow this isolate to shutdown.
      computationPort.keepIsolateAlive = false;
      return;
    }
    try {
      final FutureOr<Object?> potentiallyAsyncResult = message.computation();
      if (potentiallyAsyncResult is Future<Object?>) {
        potentiallyAsyncResult.then((Object? result) {
          sendPort.send(_ComputationResult(message.id, result, null, null));
        }, onError: (Object? e, Object? s) {
          sendPort.send(_ComputationResult(message.id, null, e, s ?? StackTrace.empty));
        });
      } else {
        sendPort.send(_ComputationResult(message.id, potentiallyAsyncResult, null, null));
      }
    } catch (e, s) {
      // If sending fails, the error becomes an uncaught error.
      sendPort.send(_ComputationResult(message.id, null, e, s));
    }
  };
  sendPort.send(_PlatformIsolateReadyMessage(computationPort.sendPort));
}

@Native<Void Function(Handle, Handle)>(
    symbol: 'PlatformIsolateNativeApi::Spawn')
external void _nativeSpawn(Function entryPoint, SendPort isolateReadyPort);

/// Returns whether the current isolate is running in the platform thread.
@Native<Bool Function()>(
    symbol: 'PlatformIsolateNativeApi::IsRunningInPlatformThread')
external bool isRunningInPlatformThread();

class _PlatformIsolateReadyMessage {
  _PlatformIsolateReadyMessage(this.computationPort);

  final SendPort computationPort;
}

class _ComputationRequest {
  _ComputationRequest(this.id, this.computation);

  final int id;
  final FutureOr<Object?> Function() computation;
}

class _ComputationResult {
  _ComputationResult(this.id, this.result, this.remoteError, this.remoteStack);

  final int id;
  final Object? result;
  final Object? remoteError;
  final Object? remoteStack;
}
